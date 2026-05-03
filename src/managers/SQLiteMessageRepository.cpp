#include "managers/SQLiteMessageRepository.h"
#include "managers/DatabaseManager.h"
#include "core/Exceptions.h"
#include "sqlite3.h"
#include <string>
#include <vector>
#include <cstdint>
using namespace std;

void SQLiteMessageRepository::throwPrepareError(const string& sql) const {
    sqlite3* db = DatabaseManager::getInstance().getConnection();
    string msg = "Failed to prepare statement. SQL: " + sql +
        " | Error: " + sqlite3_errmsg(db);
    throw DatabaseException(msg);
}

Message SQLiteMessageRepository::buildMessageFromRow(sqlite3_stmt* stmt) const {
    int messageID = sqlite3_column_int(stmt, 0);
    int senderID = sqlite3_column_int(stmt, 1);
    int receiverID = sqlite3_column_int(stmt, 2);

    const void* payloadPtr = sqlite3_column_blob(stmt, 3);
    int payloadBytes = sqlite3_column_bytes(stmt, 3);
    vector<uint8_t> payloadBlob;
    if (payloadPtr != nullptr && payloadBytes > 0) {
        const uint8_t* p = static_cast<const uint8_t*>(payloadPtr);
        payloadBlob.assign(p, p + payloadBytes);
    }
    size_t payloadBits = static_cast<size_t>(sqlite3_column_int64(stmt, 4));
    const void* treePtr = sqlite3_column_blob(stmt, 5);
    int treeBytes = sqlite3_column_bytes(stmt, 5);
    vector<uint8_t> treeBlob;
    if (treePtr != nullptr && treeBytes > 0) {
        const uint8_t* p = static_cast<const uint8_t*>(treePtr);
        treeBlob.assign(p, p + treeBytes);
    }
    size_t treeBits = static_cast<size_t>(sqlite3_column_int64(stmt, 6));
    const char* timestamp = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
    int isReadInt = sqlite3_column_int(stmt, 8);
    Message m;
    m.setMessageID(messageID);
    m.setSenderID(senderID);
    m.setReceiverID(receiverID);
    m.setPayloadBlob(payloadBlob);
    m.setPayloadBits(payloadBits);
    m.setTreeBlob(treeBlob);
    m.setTreeBits(treeBits);
    m.setTimestamp(timestamp ? timestamp : "");
    m.setIsRead(isReadInt != 0);
    return m;
}
void SQLiteMessageRepository::saveMessage(Message& msg) {
    sqlite3* db = DatabaseManager::getInstance().getConnection();
    const string sql =
        "INSERT INTO messages (senderID, receiverID, payloadBlob, payloadBits, "
        "                      treeBlob, treeBits, timestamp, isRead) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        throwPrepareError(sql);
    }
    const vector<uint8_t>& payload = msg.getPayloadBlob();
    const vector<uint8_t>& tree = msg.getTreeBlob();
    sqlite3_bind_int(stmt, 1, msg.getSenderID());
    sqlite3_bind_int(stmt, 2, msg.getReceiverID());
    if (payload.empty()) {
        sqlite3_bind_zeroblob(stmt, 3, 0);
    }
    else {
        sqlite3_bind_blob(stmt, 3, payload.data(), static_cast<int>(payload.size()), SQLITE_TRANSIENT);
    }
    sqlite3_bind_int64(stmt, 4, static_cast<sqlite3_int64>(msg.getPayloadBits()));
    if (tree.empty()) {
        sqlite3_bind_zeroblob(stmt, 5, 0);
    }
    else {
        sqlite3_bind_blob(stmt, 5,
            tree.data(),
            static_cast<int>(tree.size()),
            SQLITE_TRANSIENT);
    }
    sqlite3_bind_int64(stmt, 6, static_cast<sqlite3_int64>(msg.getTreeBits()));
    sqlite3_bind_text(stmt, 7, msg.getTimestamp().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 8, msg.getIsRead() ? 1 : 0);
    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        string err = sqlite3_errmsg(db);
        int extended = sqlite3_extended_errcode(db);
        sqlite3_finalize(stmt);
        if (rc == SQLITE_CONSTRAINT) {
            if (extended == SQLITE_CONSTRAINT_UNIQUE) {
                throw DuplicateEntryException(
                    "Message unique constraint: " + err);
            }
            throw ValidationException("Message constraint failed: " + err);
        }
        throw DatabaseException("Failed to insert message: " + err);
    }
    int newID = static_cast<int>(sqlite3_last_insert_rowid(db));
    sqlite3_finalize(stmt);
    msg.setMessageID(newID);
}

bool SQLiteMessageRepository::markAsRead(int messageID) {
    sqlite3* db = DatabaseManager::getInstance().getConnection();
    const string sql = "UPDATE messages SET isRead = 1 WHERE messageID = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        throwPrepareError(sql);
    }
    sqlite3_bind_int(stmt, 1, messageID);
    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        string err = sqlite3_errmsg(db);
        sqlite3_finalize(stmt);
        throw DatabaseException("Failed to mark message as read: " + err);
    }
    int changed = sqlite3_changes(db);
    sqlite3_finalize(stmt);
    return changed > 0;
}

bool SQLiteMessageRepository::deleteMessage(int messageID) {
    sqlite3* db = DatabaseManager::getInstance().getConnection();
    const string sql = "DELETE FROM messages WHERE messageID = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        throwPrepareError(sql);
    }
    sqlite3_bind_int(stmt, 1, messageID);
    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        string err = sqlite3_errmsg(db);
        sqlite3_finalize(stmt);
        throw DatabaseException("Failed to delete message: " + err);
    }
    int changed = sqlite3_changes(db);
    sqlite3_finalize(stmt);
    return changed > 0;
}

Message SQLiteMessageRepository::findMessageByID(int messageID) const {
    sqlite3* db = DatabaseManager::getInstance().getConnection();
    const string sql =
        "SELECT messageID, senderID, receiverID, payloadBlob, payloadBits, "
        "       treeBlob, treeBits, timestamp, isRead "
        "FROM messages WHERE messageID = ?;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        throwPrepareError(sql);
    }
    sqlite3_bind_int(stmt, 1, messageID);
    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        Message m = buildMessageFromRow(stmt);
        sqlite3_finalize(stmt);
        return m;
    }
    sqlite3_finalize(stmt);
    if (rc == SQLITE_DONE) {
        throw MessageNotFoundException(
            "No message with messageID=" + to_string(messageID));
    }
    throw DatabaseException(
        "Failed to query message by ID: " + string(sqlite3_errmsg(db)));
}
DataList<Message>
SQLiteMessageRepository::findConversation(int userA, int userB) const {
    sqlite3* db = DatabaseManager::getInstance().getConnection();
    const string sql =
        "SELECT messageID, senderID, receiverID, payloadBlob, payloadBits, "
        "       treeBlob, treeBits, timestamp, isRead "
        "FROM messages "
        "WHERE (senderID = ? AND receiverID = ?) "
        "   OR (senderID = ? AND receiverID = ?) "
        "ORDER BY timestamp ASC, messageID ASC;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        throwPrepareError(sql);
    }
    sqlite3_bind_int(stmt, 1, userA);
    sqlite3_bind_int(stmt, 2, userB);
    sqlite3_bind_int(stmt, 3, userB);
    sqlite3_bind_int(stmt, 4, userA);
    DataList<Message> result;
    int rc;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        result.add(buildMessageFromRow(stmt));
    }
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) {
        throw DatabaseException(
            "Failed to query conversation: " + string(sqlite3_errmsg(db)));
    }
    return result;
}
DataList<Message>
SQLiteMessageRepository::findInbox(int receiverID) const {
    sqlite3* db = DatabaseManager::getInstance().getConnection();
    const string sql =
        "SELECT messageID, senderID, receiverID, payloadBlob, payloadBits, "
        "       treeBlob, treeBits, timestamp, isRead "
        "FROM messages WHERE receiverID = ? "
        "ORDER BY timestamp DESC, messageID DESC;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        throwPrepareError(sql);
    }
    sqlite3_bind_int(stmt, 1, receiverID);
    DataList<Message> result;
    int rc;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        result.add(buildMessageFromRow(stmt));
    }
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) {
        throw DatabaseException(
            "Failed to query inbox: " + string(sqlite3_errmsg(db)));
    }
    return result;
}
DataList<Message>
SQLiteMessageRepository::findAllMessages() const {
    sqlite3* db = DatabaseManager::getInstance().getConnection();
    const string sql =
        "SELECT messageID, senderID, receiverID, payloadBlob, payloadBits, "
        "       treeBlob, treeBits, timestamp, isRead "
        "FROM messages ORDER BY messageID ASC;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        throwPrepareError(sql);
    }
    DataList<Message> result;
    int rc;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        result.add(buildMessageFromRow(stmt));
    }
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) {
        throw DatabaseException(
            "Failed to query all messages: " + string(sqlite3_errmsg(db)));
    }
    return result;
}
int SQLiteMessageRepository::countUnread(int receiverID) const {
    sqlite3* db = DatabaseManager::getInstance().getConnection();

    const string sql =
        "SELECT COUNT(*) FROM messages "
        "WHERE receiverID = ? AND isRead = 0;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        throwPrepareError(sql);
    }
    sqlite3_bind_int(stmt, 1, receiverID);
    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        string err = sqlite3_errmsg(db);
        sqlite3_finalize(stmt);
        throw DatabaseException("Failed to count unread messages: " + err);
    }
    int count = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);
    return count;
}