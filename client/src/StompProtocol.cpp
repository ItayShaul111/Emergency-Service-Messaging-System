#include "StompProtocol.h"
#include "event.h"
#include <string>
#include <iostream>
#include <map>
#include <vector>
#include <filesystem>
#include <fstream>


StompProtocol::~StompProtocol() {
    if (connectionHandler != nullptr) {
        connectionHandler->close();
        delete connectionHandler;
        connectionHandler = nullptr;
    }

    isConnected = false;
    shouldTerminate = true;

    summarizeMap.clear();
    channelToSubscriptionId.clear();
}

StompProtocol::StompProtocol(ConnectionHandler *connectionHandler)
    : connectionHandler(connectionHandler),        
      tempUsername(""),                           
      currentUser(""),                             
      isConnected(false),                          
      subscriptionIDGenerator(1),                  
      receiptIDGenerator(1),                       
      disconnectReceipt(-2),                       
      summarizeMap(),                              
      channelToSubscriptionId(),
      recieptIDtoAction(),
      recieptIDtoActionMutex(),                   
      summarizeMapMutex(),                         
      shouldTerminate(false)                       
{}
/********************************* HELPER METHODS ********************************************************/
string StompProtocol::createFrameString(const string &command, const string &headers, const string &body)
{
    string frame = command + "\n" + headers + "\n";
    //Checks if there's body.
    if (!body.empty()){
        frame += body + "\n"; 
    }
    frame += "\0";
    return frame;
}

string StompProtocol::addHeader(const string& key, const string& value) {
    return key + ":" + value + "\n";
}

/************************************* FRAME MAKERS *******************************************************/

// CONNNECT FRAME
string StompProtocol::makeConnectFrame(const string& login, const string& passcode) {
    string headers = addHeader("accept-version", "1.2");
    headers += addHeader("host", "stomp.cs.bgu.ac.il");
    headers += addHeader("login", login);
    headers += addHeader("passcode", passcode);

    return createFrameString("CONNECT", headers);
}

// DISCONNECT FRAME
string StompProtocol::makeDisconnectFrame(const string& receiptID) {
    string headers = addHeader("receipt", receiptID);

    return createFrameString("DISCONNECT", headers);
}

// SUBSCRIBE FRAME
string StompProtocol::makeSubscribeFrame(const string& destination, const string& subscriptionID, const string& receiptID) {
    string headers = addHeader("destination", destination);
    headers += addHeader("id", subscriptionID);
    headers += addHeader("receipt", receiptID);

    return createFrameString("SUBSCRIBE", headers);
}

// UNSUBSCRIBE FRAME
string StompProtocol::makeUnsubscribeFrame(const string& subscriptionID, const string& receiptID) {
    string headers = addHeader("id", subscriptionID);
    headers += addHeader("receipt", receiptID);

    return createFrameString("UNSUBSCRIBE", headers);
}

// SEND FRAME
string StompProtocol::makeSendFrame(const string& destination, Event eventToSend) {
    string headers = addHeader("destination", destination);
    
    string body = addHeader("user", " " + eventToSend.getEventOwnerUser());
    body += addHeader("city", " " + eventToSend.get_city());
    body += addHeader("event name", " " + eventToSend.get_name());
    body += addHeader("date time", " " + to_string(eventToSend.get_date_time()));
    body += "general information:\n";

    for (const auto &pair : eventToSend.get_general_information()) {
        body += /*"     " +*/ pair.first + ": " + pair.second + "\n";
    }

    body += "description:\n" + eventToSend.get_description();

    return createFrameString("SEND", headers, body);
}

/************************************* HANDLER FUNCTIONS *******************************************************/

// HANDLE LOGIN
string StompProtocol::handleLogin(const string &hostPort, const string &username, const string &password) {

    if (isConnected) {
        cout << "The client is already logged in, log out before trying again." << endl;
        return "";
    }



    size_t colonPos = hostPort.find(':');
    if (colonPos == string::npos) {
        cout << "Invalid host:port format" << endl;
        return "";
    }

    string host = hostPort.substr(0, colonPos);
    string port = hostPort.substr(colonPos + 1);

    if (host.empty() || username.empty() || password.empty()) {
        cout << "Host, username, or password cannot be empty" << endl;
        return "";
    }

    connectionHandler = new ConnectionHandler(host, static_cast<short>(stoi(port)));
    if(!connectionHandler->connect()){
        cout << "Couldn't connect to server" << endl;
        return "";
    }
    
    shouldTerminate = false;
    thread serverThread = thread([this]() {serverThreadLoop(); });
    serverThread.detach();

    string connectFrame =  makeConnectFrame(username, password);
    tempUsername = username;
    return connectFrame;
}

// HANDLE LOGOUT 
string StompProtocol::handleLogout() {

    if(!isConnected) {
        cout << "User isn't logged in" << endl;
        return "";
    }

    disconnectReceipt = receiptIDGenerator;
    receiptIDGenerator++;

    string disconnectFrame = makeDisconnectFrame(to_string(disconnectReceipt));
    return disconnectFrame;
}

// HANDLE JOIN 
string StompProtocol::handleJoin(const string &topic) {

    if (!isConnected)  {
        cout << "User is not logged in, log in before trying to join " + topic << endl;
        return "";
    }

    lock_guard<mutex> lock(recieptIDtoActionMutex);
    recieptIDtoAction[receiptIDGenerator] = "Joined channel " + topic;
    channelToSubscriptionId[topic] = subscriptionIDGenerator;
    string subscribeFrame = makeSubscribeFrame(topic, to_string(subscriptionIDGenerator), to_string(receiptIDGenerator));
    receiptIDGenerator++;
    subscriptionIDGenerator++;

    return subscribeFrame;
}                                                    

// HANDLE EXIT
string StompProtocol::handleExit(const string &topic) {

    if (!isConnected) {
        cout << "User is not logged in, log in before trying to exit from: " + topic << endl;
        return "";
    }

    if (channelToSubscriptionId.find(topic) == channelToSubscriptionId.end()) {
        cout << "User is not subbed to: " + topic << endl;
        return "";
    }
    
    lock_guard<mutex> lock(recieptIDtoActionMutex);
    recieptIDtoAction[receiptIDGenerator] = "Exited channel " + topic;
    int subscriptionIDforTopic = channelToSubscriptionId[topic];
    string unsubscribeFrame = makeUnsubscribeFrame(to_string(subscriptionIDforTopic), to_string(receiptIDGenerator));
    receiptIDGenerator++;

    channelToSubscriptionId.erase(topic);

    return unsubscribeFrame;
}             

// HANDLE REPORT
string StompProtocol::handleReport(const string& file) {

    if (!isConnected)  {
        cout << "User is not logged in, log in before trying to report" << endl;
        return "";
    }

    names_and_events jsonData = parseEventsFile(file);
    const string &channelName = jsonData.channel_name;

    vector<Event> &events = jsonData.events;
    
    if (events.empty()) {
        return "";
    }

    for (auto &eventToSend : events) {
        eventToSend.setEventOwnerUser(currentUser);
        saveEventForSummarize(channelName, eventToSend);
        string sendFrame = makeSendFrame(channelName, eventToSend);
        connectionHandler->sendFrameAscii(sendFrame, '\0');
    }
    return "";
}

void StompProtocol::saveEventForSummarize(const string& channelName, const Event& event) {
    lock_guard<mutex> lock(summarizeMapMutex);
    if (summarizeMap.find(event.getEventOwnerUser()) == summarizeMap.end()) {
        summarizeMap[event.getEventOwnerUser()] = map<string, vector<Event>>();
    }
    if (summarizeMap[event.getEventOwnerUser()].find(channelName) == summarizeMap[event.getEventOwnerUser()].end()){
        summarizeMap[event.getEventOwnerUser()][channelName] = vector<Event>();
    }
    summarizeMap[event.getEventOwnerUser()][channelName].push_back(event);
}

void StompProtocol::createSummary(const string &channelName, const string &user, const string &file) {


    if (!isConnected)  {
        cout << "User is not logged in, log in before trying to create a summary" << endl;
        return;
    }
    if (channelToSubscriptionId.find(channelName) == channelToSubscriptionId.end()) {
        cout << "User cannot create a summary because he is not subscribed to the channel: " << channelName << endl;
        return;
    }
    lock_guard<mutex> lock(summarizeMapMutex);
    
    // if (summarizeMap.find(user) == summarizeMap.end()) {
    //     cout << "Can't summarize, the given user isn't didn't send any message relative to the given channel"<< endl;
    //     return;
    // }

    if (summarizeMap[user].find(channelName) == summarizeMap[channelName].end()) {
        cout << "Can't summarize, the given channel isn't exist" << endl;
        return;
    }
    
    vector<Event> &events = summarizeMap[user][channelName];
    // Sorting first by date time then by name.
    sort(events.begin(), events.end(), [](const Event &a, const Event &b) {
        if (a.get_date_time() != b.get_date_time()) {
            return a.get_date_time() < b.get_date_time();
        }
        return a.get_name() < b.get_name();
    });

    // Calculating stats.
    int activeCount = 0;
    int forcesArrivalCount = 0;
    for (const auto &ev : events) {
        if (ev.get_general_information().at("active") == "true") {
            activeCount++;
        }
        if (ev.get_general_information().at("forces_arrival_at_scene") == "true") {
            forcesArrivalCount++;
        }
    }

    // Creating file if isn't existing.
    ofstream outFile(file, ios::trunc);
    if (!outFile.is_open()) {
        cout << "Error: Could not open or create the file: " << file << endl;
        return;
    }
    
    // Writing the summary to the file
    outFile << "Channel: " << channelName << endl;
    outFile << "Stats:" << endl;
    outFile << "Total: " << events.size() << endl;
    outFile << "active: " << activeCount << endl;
    outFile << "forces arrival at scene: " << forcesArrivalCount << "\n" << endl;
    outFile << "Event Reports:" << endl;


    // Writing all reports.
    int reportNumber = 1;
    for (const auto &ev : events) {
        outFile << "Report_" << reportNumber << ":" << endl;
        outFile << "   city: " << ev.get_city() << endl;

        time_t epochTime = static_cast<time_t>(ev.get_date_time());
        outFile << "   date time: " << epochToDate(epochTime) << endl;
        outFile << "   event name: " << ev.get_name() << endl;
        
        string descriptionSummary = ev.get_description();
        if (descriptionSummary.length() > 27) {
            descriptionSummary = descriptionSummary.substr(0, 27) + "...";
        }
        outFile << "   summary: " << descriptionSummary << endl;
        reportNumber++;
    }
    outFile.close();
    cout << "Summary created successfully in file: " << file << endl;
}

string StompProtocol:: epochToDate(time_t epochTime) {
    struct tm *timeInfo = localtime(&epochTime);

    
    char buffer[20];
    strftime(buffer, sizeof(buffer), "%d/%m/%y %H:%M", timeInfo);

    return string(buffer);
}




void StompProtocol::initiate() {
    thread clientThread = thread([this]() {clientThreadLoop(); });
    // thread serverThread = thread([this]() {serverThreadLoop(); });

    clientThread.join();
    //serverThread.join();
}

void StompProtocol::clientThreadLoop() {
    while (true) {
        string line;
        getline(cin, line);
        vector<string> tokens = splitString(line, ' ');
        string frameToSend;
        if (tokens.empty()) continue;

        try {
            if (tokens[0] == "login") {
                if (tokens.size() != 4) {
                    cout << "Expecting: login <host:port> <username> <password>" << endl;
                    continue;
                }
                frameToSend = handleLogin(tokens[1], tokens[2], tokens[3]);

            } else if (tokens[0] == "exit") {
                if (tokens.size() != 2) {
                    cout << "Expecting: exit <topic>" << endl;
                    continue;
                }
                frameToSend = handleExit(tokens[1]);
            } 
            else if (tokens[0] == "join") {
                if (tokens.size() != 2) {
                    cout << "Expecting: join <topic>" << endl;
                    continue;
                }
                frameToSend = handleJoin(tokens[1]);
            }
            else if (tokens[0] == "report") {
                if (tokens.size() != 2) {
                    cout << "Expecting: report <file>" << endl;
                    continue;
                }
                frameToSend = handleReport(tokens[1]);
            } else if (tokens[0] == "logout") {
                frameToSend = handleLogout();
            } else if (tokens[0] == "summary") {
                if (tokens.size() != 4) {
                    cout << "Expecting: summary <channel_name> <user> <file>" << endl;
                    continue;
                }
                createSummary(tokens[1], tokens[2], tokens[3]);
                continue;
            } else {
                cout << "Unknown command" << endl;
                continue;
            }

            if (!frameToSend.empty()) {
                if (!connectionHandler->sendFrameAscii(frameToSend, '\0')) {
                    connectionHandler->close();
                    cerr << "Failed to send frame: " << endl;
                }
            }

        } catch (const exception& e) {
            cerr << "Error: " << e.what() << endl;
        }
    }
}


void StompProtocol::serverThreadLoop() {
    while (!shouldTerminate) {
        string responseFromServer;
        if (connectionHandler != nullptr){
            if (connectionHandler->getFrameAscii(responseFromServer, '\0')) {
                vector<string> tokens = splitString(responseFromServer, '\n');
                string command = tokens[0];
                if (command == "CONNECTED") {
                    isConnected = true;
                    currentUser = tempUsername;
                    cout << "Login successful" << endl;
                } else if (command == "RECEIPT") {
                    int receiptId = extractReceiptId(responseFromServer);
                    if (receiptId == disconnectReceipt) {
                        disconnect();
                        cout << "Logged out successfully" << endl;
                    } else {
                        lock_guard<mutex> lock(recieptIDtoActionMutex);
                        if (recieptIDtoAction.find(receiptId) != recieptIDtoAction.end()) {
                            cout << recieptIDtoAction[receiptId] << endl;
                        }
                    }
                } else if (command == "ERROR") {
                    cout << "Error recieved from server:" << endl;
                    cout << responseFromServer << endl;
                    disconnect();
                } else if (command == "MESSAGE") {
                    Event event = createEvent(responseFromServer);
                    if(event.getEventOwnerUser() != currentUser) {
                        saveEventForSummarize(event.get_channel_name() ,event);
                    }  
                } else {
                    cout << "Unexpected frame received: " << command << endl;
                }
                // cout << responseFromServer << endl;
            }
        }
        
    }
}

int StompProtocol::extractReceiptId(const string &frame) {
    size_t receiptPos = frame.find("receipt-id:");
    if (receiptPos != string::npos) {
        size_t start = receiptPos + string("receipt-id:").length();
        size_t end = frame.find('\n', start); 

        if (end != string::npos) {
            try {
                return stoi(frame.substr(start, end - start));
            } catch (const invalid_argument &e) {
                cerr << "Invalid receipt-id format in frame: " << e.what() << endl;
            } catch (const out_of_range &e) {
                cerr << "Receipt-id out of range in frame: " << e.what() << endl;
            }
        }
    }
    return -1;
}

void StompProtocol::disconnect() {
    connectionHandler->close();
    connectionHandler = nullptr;
    tempUsername = "";
    currentUser = "";
    isConnected = false;
    subscriptionIDGenerator = 1;
    receiptIDGenerator = 1;
    disconnectReceipt = -2;
    shouldTerminate = true;

    channelToSubscriptionId.clear();
    summarizeMap.clear();
    
}

Event StompProtocol::createEvent(const string &frame) {
    string channel_name, city, name, description;
    int date_time = 0;
    map<string, string> general_information;

    // Split the frame into lines
    vector<string> lines = splitString(frame, '\n');
    bool isDescription = false; // To identify if we're processing the description

    string username;

    for (const string &line : lines) {
        if (line.empty()) {
            // Skip empty lines
            continue;
        }

        if (line.find("destination:") == 0) {
            // Extract channel name from destination
            channel_name = line.substr(13); // Skip "destination:"
        } else if (line.find("user:") == 0){
            username = line.substr(6);
        } else if (line.find("city:") == 0) {
            city = line.substr(6); // Skip "city:"
        } else if (line.find("event name:") == 0) {
            name = line.substr(12); // Skip "event name:"
        } else if (line.find("date time:") == 0) {
            date_time = stoi(line.substr(10)); // Convert "date time:" value to integer
        } else if (line.find("general information:") == 0) {
            // Start parsing general information
            isDescription = false; // Reset description flag
        } else if (line.find("description:") == 0) {
            isDescription = true; // Start parsing the description
            description = line.substr(12); // Initialize description with the first line
        } else if (isDescription) {
            // Append to description
            if (!description.empty()) {
                description += "\n"; // Add newline for subsequent lines
            }
            description += line;
        } else if(line.find("subscription:") == 0 || line.find("message-id:") == 0){
            continue;
        } else if (line.find(':') != string::npos) {
            // Parse key-value pairs in general information
            size_t delimiterPos = line.find(':');
            string key = line.substr(0, delimiterPos);
            string value = line.substr(delimiterPos + 2);
            general_information[key] = value;
        }
    }

    // Create and return the Event object
    Event event = Event(channel_name, city, name, date_time, description, general_information);
    event.setEventOwnerUser(username);
    return event;
}

vector<string> StompProtocol::splitString(const string &str, char delimiter) {
    vector<string> tokens;
    string token;
    istringstream tokenStream(str);
    while (getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}