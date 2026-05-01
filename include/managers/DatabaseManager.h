#ifndef SKILLBRIDGE_DATABASEMANAGER_H
#define SKILLBRIDGE_DATABASEMANAGER_H

#include <string>

struct sqlite3;

class DatabaseManager 
{
private:
    sqlite3* db;
    std::string dbPath;
    bool isOpen;

    DatabaseManager();

    ~DatabaseManager();

public:
    DatabaseManager(const DatabaseManager&) = delete;

    DatabaseManager& operator=(const DatabaseManager&) = delete;

    static DatabaseManager& getInstance();

    void open(const std::string& path);

    void close();

    sqlite3* getConnection();

    void execute(const std::string& sql);

    void initializeSchema();

    bool isConnected() const 
    {
        return isOpen; 
    }
};

#endif