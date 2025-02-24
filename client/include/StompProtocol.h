#pragma once

#include "ConnectionHandler.h"
#include <string>
#include <map>
#include <vector>
#include <mutex>
#include "event.h"

using namespace std;

class StompProtocol {

private:

    ConnectionHandler *connectionHandler;   
    string tempUsername;
    string currentUser;                        
    bool isConnected;                         
    
    int subscriptionIDGenerator;                 
    int receiptIDGenerator;                      

    int disconnectReceipt;                      

    map<string, map<string, vector <Event>>> summarizeMap;      // USER ---->> (CHANNEL, vector of CORESSPONDING EVENTS)
    map<string, int> channelToSubscriptionId; // Channel --> SubscriptionID
    map<int, string> recieptIDtoAction; 
    mutex recieptIDtoActionMutex; 
    mutex summarizeMapMutex; 
    bool shouldTerminate;


    // HELPER METHODS 
    string createFrameString(const string &command, const string &headers, const string &body = "");
    string addHeader(const string& key, const string& value);

public:

    StompProtocol(ConnectionHandler *connectionHandler);
    ~StompProtocol();
    StompProtocol(const StompProtocol&) = delete;
    StompProtocol& operator= (const StompProtocol&) = delete;

    string handleLogin(const string &hostPort, const string &username, const string &password); 
    string handleLogout();                                                                      
    string handleJoin(const string &topic);                                                     
    string handleExit(const string &topic);    

    string handleReport(const string &filepath);
    void saveEventForSummarize(const string& channelName, const Event& event);
    
    void createSummary(const string &channelName, const string &user, const string &file);

    string epochToDate(time_t epochTime);

    string makeConnectFrame(const string& login, const string& passcode);
    string makeDisconnectFrame(const string& recieptID);
    string makeSubscribeFrame(const string& destination, const string& subscriptionID, const string& receiptID);
    string makeUnsubscribeFrame(const string& subscriptionID, const string& receiptID);
    string makeSendFrame(const string& destination, Event eventToSend);

    void initiate();
    void clientThreadLoop();
    void serverThreadLoop();
    int extractReceiptId(const string& frame);
    void disconnect();
    Event createEvent(const string& frame);
    
    //Helper method
    vector<string> splitString(const string &str, char delimiter); 
};