#pragma once
#include <iostream>
#include <string>
#include <ctime>
#include "sqlite/sqlite3.h"
using namespace std;
const int MAX_MESSAGES = 200;
const string DB_FILE = "skillbridge.db";
class Message {
    int messageID;
    int senderID;
    int receiverID;
    string content;
    string timestamp;
    static sqlite3* db;

    string getCurrentTime();
    void runSQL(const string& sql);
public:
    Message();
    ~Message();
    void sendMessage(int senderID, int receiverID, string msg);
    Message* viewMessages(int userID, int& count);
    int getMessageID();
    int getSenderID();
    int getReceiverID();
    string getContent();
    string getTimestamp();
    void display();
};