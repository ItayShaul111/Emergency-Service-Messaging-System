#pragma once

#include <string>
#include <iostream>
#include <map>
#include <vector>

using namespace std; 

class Event
{
private:
    // name of channel
    string channel_name;
    // city of the event 
    string city;
    // name of the event
    string name;
    // time of the event in seconds
    int date_time;
    // description of the event
    string description;
    // map of all the general information
    map<string, string> general_information;
    string eventOwnerUser;

public:
    Event(string channel_name, string city, string name, int date_time, string description, map<string, string> general_information);
    Event(const string & frame_body);
    virtual ~Event();
    void setEventOwnerUser(string setEventOwnerUser);
    const string &getEventOwnerUser() const;
    const string &get_channel_name() const;
    const string &get_city() const;
    const string &get_description() const;
    const string &get_name() const;
    int get_date_time() const;
    const std::map<std::string, std::string> &get_general_information() const; 
    void split_str(const std::string& str, char delimiter, std::vector<std::string>& result);
};

// an object that holds the names of the teams and a vector of events, to be returned by the parseEventsFile function
struct names_and_events {
    string channel_name;
    vector<Event> events;
};

// function that parses the json file and returns a names_and_events object
names_and_events parseEventsFile(string json_path);
