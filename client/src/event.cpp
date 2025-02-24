#include "../include/event.h"
#include "../include/json.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <cstring>


using namespace std;
using json = nlohmann::json;

Event::Event(string channel_name, string city, string name, int date_time,
             string description, map<string, string> general_information)
    : channel_name(channel_name), city(city), name(name),
      date_time(date_time), description(description), general_information(general_information), eventOwnerUser("")
{
}

Event::~Event()
{
}

void Event::setEventOwnerUser(string setEventOwnerUser) {
    eventOwnerUser = setEventOwnerUser;
}

const string &Event::getEventOwnerUser() const {
    return eventOwnerUser;
}

const string &Event::get_channel_name() const
{
    return this->channel_name;
}

const string &Event::get_city() const
{
    return this->city;
}

const string &Event::get_name() const
{
    return this->name;
}

int Event::get_date_time() const
{
    return this->date_time;
}

const map<string, string> &Event::get_general_information() const
{
    return this->general_information;
}

const string &Event::get_description() const
{
    return this->description;
}

Event::Event(const string &frame_body): channel_name(""), city(""), 
                                             name(""), date_time(0), description(""), general_information(),
                                             eventOwnerUser("")
{
    stringstream ss(frame_body);
    string line;
    string eventDescription;
    map<string, string> general_information_from_string;
    bool inGeneralInformation = false;
    while(getline(ss,line,'\n')){
        vector<string> lineArgs;
        if(line.find(':') != string::npos) {
            split_str(line, ':', lineArgs);
            string key = lineArgs.at(0);
            string val;
            if(lineArgs.size() == 2) {
                val = lineArgs.at(1);
            }
            if(key == "user") {
                eventOwnerUser = val;
            }
            if(key == "channel name") {
                channel_name = val;
            }
            if(key == "city") {
                city = val;
            }
            else if(key == "event name") {
                name = val;
            }
            else if(key == "date time") {
                date_time = stoi(val);
            }
            else if(key == "general information") {
                inGeneralInformation = true;
                continue;
            }
            else if(key == "description") {
                while(getline(ss,line,'\n')) {
                    eventDescription += line + "\n";
                }
                description = eventDescription;
            }

            if(inGeneralInformation) {
                general_information_from_string[key.substr(1)] = val;
            }
        }
    }
    general_information = general_information_from_string;
}

names_and_events parseEventsFile(string json_path)
{
    ifstream f(json_path);
    json data = json::parse(f);

    string channel_name = data["channel_name"];

    // run over all the events and convert them to Event objects
    vector<Event> events;
    for (auto &event : data["events"])
    {
        string name = event["event_name"];
        string city = event["city"];
        int date_time = event["date_time"];
        string description = event["description"];
        map<string, string> general_information;
        for (auto &update : event["general_information"].items())
        {
            if (update.value().is_string())
                general_information[update.key()] = update.value();
            else
                general_information[update.key()] = update.value().dump();
        }

        events.push_back(Event(channel_name, city, name, date_time, description, general_information));
    }
    names_and_events events_and_names{channel_name, events};

    return events_and_names;
}

/*****************************************HELPER METHODS*******************************************/
void Event::split_str(const string& line, char delimiter, vector<string>& lineArgs){
    stringstream ss(line);
    string curr;
    while (getline(ss, curr, delimiter)) {
        lineArgs.push_back(curr);
    }
}

