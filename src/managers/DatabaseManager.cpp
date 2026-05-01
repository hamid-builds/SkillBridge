#include "managers/DatabaseManager.h"
#include "core/Exceptions.h"
#include "sqlite3.h"

using namespace std;

DatabaseManager::DatabaseManager() : db(nullptr), dbPath(""), isOpen(false)
{

}

DatabaseManager::~DatabaseManager() 
{
    close();
}

DatabaseManager& DatabaseManager::getInstance() 
{
    static DatabaseManager instance;
    return instance;
}

void DatabaseManager::open(const string& path) 
{
    if (isOpen) 
    {
        if (path == dbPath) 
            return;
        throw DatabaseException("Database is already open at " + dbPath);
    }

    int rc = sqlite3_open(path.c_str(), &db);
    if (rc != SQLITE_OK) 
    {
        string msg = "Failed to open database at " + path + ": " + (db ? sqlite3_errmsg(db) : "unknown error");
        if (db) 
        {
            sqlite3_close(db);
            db = nullptr;
        }
        throw DatabaseException(msg);
    }

    dbPath = path;

    isOpen = true;

    execute("PRAGMA foreign_keys = ON;");

    initializeSchema();
}

void DatabaseManager::close() 
{
    if (!isOpen) return;
    sqlite3_close(db);
    db = nullptr;
    isOpen = false;
    dbPath = "";
}

sqlite3* DatabaseManager::getConnection() 
{
    if (!isOpen) 
    {
        throw DatabaseException("Database is not open. Call open() first.");
    }
    return db;
}

void DatabaseManager::execute(const string& sql) 
{
    if (!isOpen) 
    {
        throw DatabaseException("Database is not open. Call open() first.");
    }

    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) 
    {
        string msg = "SQL error: ";
        msg += (errMsg ? errMsg : "unknown");
        msg += " | Query: " + sql;
        sqlite3_free(errMsg);
        throw DatabaseException(msg);
    }
}

void DatabaseManager::initializeSchema() 
{
    execute
    (
        "CREATE TABLE IF NOT EXISTS users ("
        "  userID        INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  name          TEXT NOT NULL CHECK(length(name) > 0),"
        "  email         TEXT NOT NULL UNIQUE CHECK(length(email) > 0),"
        "  passwordHash  TEXT NOT NULL CHECK(length(passwordHash) > 0),"
        "  role          TEXT NOT NULL CHECK(role IN ('CLIENT','FREELANCER','ADMIN')),"
        "  balance       REAL NOT NULL DEFAULT 0.0 CHECK(balance >= 0),"
        "  createdAt     TEXT NOT NULL DEFAULT (datetime('now')),"
        "  portfolio     TEXT NOT NULL DEFAULT '',"
        "  skills        TEXT NOT NULL DEFAULT '',"
        "  avgRating     REAL NOT NULL DEFAULT 0.0 "
        "                CHECK(avgRating >= 0 AND avgRating <= 5)"
        ");"
    );

}