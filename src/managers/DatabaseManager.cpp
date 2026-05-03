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
    execute(
        "CREATE TABLE IF NOT EXISTS gigs ("
        "  gigID        INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  ownerID      INTEGER NOT NULL,"
        "  title        TEXT NOT NULL "
        "               CHECK(length(title) >= 3 AND length(title) <= 100),"
        "  description  TEXT NOT NULL "
        "               CHECK(length(description) >= 10 AND length(description) <= 2000),"
        "  price        REAL NOT NULL "
        "               CHECK(price > 0 AND price < 1000000),"
        "  category     TEXT NOT NULL "
        "               CHECK(category IN ('DESIGN','WRITING','CODING','MARKETING','TUTORING','OTHER')),"
        "  isActive     INTEGER NOT NULL DEFAULT 1 CHECK(isActive IN (0, 1)),"
        "  createdAt    TEXT NOT NULL DEFAULT (datetime('now')),"
        "  FOREIGN KEY(ownerID) REFERENCES users(userID) ON DELETE CASCADE"
        ");"
    );
   
    execute(
        "CREATE TABLE IF NOT EXISTS orders ("
        "  orderID      INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  gigID        INTEGER NOT NULL,"
        "  buyerID      INTEGER NOT NULL,"
        "  sellerID     INTEGER NOT NULL,"
        "  amount       REAL NOT NULL "
        "               CHECK(amount > 0 AND amount < 1000000),"
        "  status       TEXT NOT NULL "
        "               CHECK(status IN ('PENDING','IN_PROGRESS','COMPLETED','CANCELLED')),"
        "  placedAt     TEXT NOT NULL CHECK(length(placedAt) > 0),"
        "  completedAt  TEXT,"
        "  deadline     TEXT NOT NULL CHECK(length(deadline) > 0),"
        "  CHECK(buyerID <> sellerID),"
        "  FOREIGN KEY(gigID)    REFERENCES gigs(gigID)    ON DELETE CASCADE,"
        "  FOREIGN KEY(buyerID)  REFERENCES users(userID)  ON DELETE CASCADE,"
        "  FOREIGN KEY(sellerID) REFERENCES users(userID)  ON DELETE CASCADE"
        ");"
    );
    execute(
        "CREATE TABLE IF NOT EXISTS messages ("
        "  messageID    INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  senderID     INTEGER NOT NULL,"
        "  receiverID   INTEGER NOT NULL,"
        "  payloadBlob  BLOB    NOT NULL,"
        "  payloadBits  INTEGER NOT NULL CHECK(payloadBits >= 0),"
        "  treeBlob     BLOB    NOT NULL,"
        "  treeBits     INTEGER NOT NULL CHECK(treeBits >= 0),"
        "  timestamp    TEXT    NOT NULL CHECK(length(timestamp) > 0),"
        "  isRead       INTEGER NOT NULL DEFAULT 0 CHECK(isRead IN (0, 1)),"
        "  CHECK(senderID <> receiverID),"
        "  FOREIGN KEY(senderID)   REFERENCES users(userID) ON DELETE CASCADE,"
        "  FOREIGN KEY(receiverID) REFERENCES users(userID) ON DELETE CASCADE"
        ");"
    );
    execute(
        "CREATE INDEX IF NOT EXISTS idx_messages_pair_time "
        "ON messages (senderID, receiverID, timestamp);"
    );
}