#include "Message.h"
sqlite3* Message::db = nullptr;
Message::Message() : messageID(0), senderID(0), receiverID(0) {
    if (db != nullptr)
        return;
    if (sqlite3_open(DB_FILE.c_str(), &db) != SQLITE_OK) {
        cout << "[Error] Cannot open database.\n";
        return;
    }
    runSQL(
        "CREATE TABLE IF NOT EXISTS messages ("
        "messageID INTEGER PRIMARY KEY AUTOINCREMENT, "
        "senderID INTEGER NOT NULL, "
        "receiverID INTEGER NOT NULL, "
        "content TEXT NOT NULL, "
        "timestamp TEXT NOT NULL);"
    );
    cout << "[DB] Database ready: " << DB_FILE << "\n";
}
Message::~Message() {
    if (db) {
        sqlite3_close(db);
        db = nullptr;
    }
}
string Message::getCurrentTime() {
    time_t now = time(0);
    char   buffer[30];
    struct tm t;
#ifdef _WIN32
    localtime_s(&t, &now);
#else
    localtime_r(&now, &t);
#endif
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &t);
    return string(buffer);
}
void Message::runSQL(const string& sql) {
    char* err = nullptr;
    sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &err);
    if (err) {
        cout << "[DB Error] " << err << "\n";
        sqlite3_free(err);
    }
}
void Message::sendMessage(int sID, int rID, string msg)
{
    if (sID == rID) {
        cout << "[Error] You cannot message yourself.\n";
        return;
    }
    if (msg.empty()) {
        cout << "[Error] Message cannot be empty.\n";
        return;
    }
    if (!db) {
        cout << "[Error] Database not connected.\n";
        return;
    }
    string ts = getCurrentTime();
    string sql = "INSERT INTO messages (senderID, receiverID, content, timestamp) "
        "VALUES ("
        + to_string(sID) + ", "
        + to_string(rID) + ", '"
        + msg + "', '"
        + ts + "');";
    char* err = nullptr;
    int result = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &err);
    if (result == SQLITE_OK) {
        cout << "\n[OK] Message sent!\n";
        cout << "User " << sID << " -> User " << rID << "\n";
        cout << "Time: " << ts << "\n";
    }
    else {
        cout << "[DB Error] Could not save message: " << err << "\n";
        sqlite3_free(err);
    }
}
Message* Message::viewMessages(int userID, int& count){
    static Message results[MAX_MESSAGES];
    count = 0;
    if (!db) {
        cout << "[Error] Database not connected.\n";
        return nullptr;
    }
    string sql = "SELECT messageID, senderID, receiverID, content, timestamp "
        "FROM messages "
        "WHERE senderID = " + to_string(userID) +
        " OR receiverID = " + to_string(userID) +
        " ORDER BY messageID ASC;";
    char** table = nullptr;
    int rows = 0;
    int cols = 0;
    char* err = nullptr;
    sqlite3_get_table(db, sql.c_str(), &table, &rows, &cols, &err);
    if (err) {
        cout << "[DB Error] " << err << "\n";
        sqlite3_free(err);
        return nullptr;
    }
    for (int i = 0; i < rows && count < MAX_MESSAGES; i++) {
        int offset = (i + 1) * cols;
        results[count].messageID = atoi(table[offset + 0]);
        results[count].senderID = atoi(table[offset + 1]);
        results[count].receiverID = atoi(table[offset + 2]);
        results[count].content = string(table[offset + 3]);
        results[count].timestamp = string(table[offset + 4]);
        count++;
    }
    sqlite3_free_table(table);
    if (count == 0){
        cout << "\n[Info] No messages found for User " << userID << ".\n";
        return nullptr;
    }
    return results;
}
int Message::getMessageID() {
    return messageID;
}
int Message::getSenderID() {
    return senderID;
}
int Message::getReceiverID() {
    return receiverID;
}
string Message::getContent() {
    return content;
}
string Message::getTimestamp() {
    return timestamp;
}
void Message::display()
{
    cout << "  ----------------------------------------\n";
    cout << "  Msg #" << messageID << "\n";
    cout << "  From   : User " << senderID << "\n";
    cout << "  To     : User " << receiverID << "\n";
    cout << "  Time   : " << timestamp << "\n";
    cout << "  Message: " << content << "\n";
}