#include "../../include/managers/SQLiteEndorsementRepository.h"
#include "../../include/core/Exceptions.h"
#include <string>

SQLiteEndorsementRepository::SQLiteEndorsementRepository(sqlite3* db) : db_(db) {
    if (!db_)
        throw DatabaseException("SQLiteEndorsementRepository: null db handle");
}

Endorsement SQLiteEndorsementRepository::buildFromRow(sqlite3_stmt* stmt) {
    Endorsement e;
    e.setEndorsementID(sqlite3_column_int(stmt, 0));
    e.setFromUserID(sqlite3_column_int(stmt, 1));
    e.setToUserID(sqlite3_column_int(stmt, 2));
    e.setWeight(sqlite3_column_double(stmt, 3));
    auto text = [&](int col) -> std::string {
        const unsigned char* t = sqlite3_column_text(stmt, col);
        return t ? reinterpret_cast<const char*>(t) : "";
        };
    e.setSkill(text(4));
    e.setTimestamp(text(5));
    return e;
}

void SQLiteEndorsementRepository::saveEndorsement(Endorsement& endorsement) {
    if (exists(endorsement.getFromUserID(),
        endorsement.getToUserID(),
        endorsement.getSkill()))
        throw DuplicateEntryException(
            "endorsement already exists for this (from, to, skill) triple");

    const char* sql =
        "INSERT INTO endorsements (fromUserID, toUserID, skill, weight, timestamp) "
        "VALUES (?, ?, ?, ?, ?);";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK)
        throw DatabaseException(sqlite3_errmsg(db_));

    sqlite3_bind_int(stmt, 1, endorsement.getFromUserID());
    sqlite3_bind_int(stmt, 2, endorsement.getToUserID());
    sqlite3_bind_text(stmt, 3, endorsement.getSkill().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 4, endorsement.getWeight());
    sqlite3_bind_text(stmt, 5, endorsement.getTimestamp().c_str(), -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE)
        throw DatabaseException(sqlite3_errmsg(db_));

    endorsement.setEndorsementID(
        static_cast<int>(sqlite3_last_insert_rowid(db_)));
}

Endorsement SQLiteEndorsementRepository::findByID(int endorsementID) const {
    const char* sql =
        "SELECT endorsementID, fromUserID, toUserID, weight, skill, timestamp "
        "FROM endorsements WHERE endorsementID = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK)
        throw DatabaseException(sqlite3_errmsg(db_));
    sqlite3_bind_int(stmt, 1, endorsementID);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        Endorsement e = buildFromRow(stmt);
        sqlite3_finalize(stmt);
        return e;
    }
    sqlite3_finalize(stmt);
    throw EndorsementNotFoundException("endorsementID=" + std::to_string(endorsementID));
}

DataList<Endorsement> SQLiteEndorsementRepository::findByFrom(int fromUserID) const {
    const char* sql =
        "SELECT endorsementID, fromUserID, toUserID, weight, skill, timestamp "
        "FROM endorsements WHERE fromUserID = ? ORDER BY timestamp DESC;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK)
        throw DatabaseException(sqlite3_errmsg(db_));
    sqlite3_bind_int(stmt, 1, fromUserID);
    DataList<Endorsement> results;
    while (sqlite3_step(stmt) == SQLITE_ROW)
        results.add(buildFromRow(stmt));
    sqlite3_finalize(stmt);
    return results;
}

DataList<Endorsement> SQLiteEndorsementRepository::findByTo(int toUserID) const {
    const char* sql =
        "SELECT endorsementID, fromUserID, toUserID, weight, skill, timestamp "
        "FROM endorsements WHERE toUserID = ? ORDER BY timestamp DESC;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK)
        throw DatabaseException(sqlite3_errmsg(db_));
    sqlite3_bind_int(stmt, 1, toUserID);
    DataList<Endorsement> results;
    while (sqlite3_step(stmt) == SQLITE_ROW)
        results.add(buildFromRow(stmt));
    sqlite3_finalize(stmt);
    return results;
}

DataList<Endorsement> SQLiteEndorsementRepository::findAll() const {
    const char* sql =
        "SELECT endorsementID, fromUserID, toUserID, weight, skill, timestamp "
        "FROM endorsements ORDER BY timestamp DESC;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK)
        throw DatabaseException(sqlite3_errmsg(db_));
    DataList<Endorsement> results;
    while (sqlite3_step(stmt) == SQLITE_ROW)
        results.add(buildFromRow(stmt));
    sqlite3_finalize(stmt);
    return results;
}

bool SQLiteEndorsementRepository::deleteEndorsement(int endorsementID) {
    const char* sql = "DELETE FROM endorsements WHERE endorsementID = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK)
        throw DatabaseException(sqlite3_errmsg(db_));
    sqlite3_bind_int(stmt, 1, endorsementID);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return sqlite3_changes(db_) > 0;
}

bool SQLiteEndorsementRepository::exists(int fromUserID, int toUserID,
    const std::string& skill) const {
    const char* sql =
        "SELECT 1 FROM endorsements "
        "WHERE fromUserID = ? AND toUserID = ? AND skill = ? LIMIT 1;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK)
        throw DatabaseException(sqlite3_errmsg(db_));
    sqlite3_bind_int(stmt, 1, fromUserID);
    sqlite3_bind_int(stmt, 2, toUserID);
    sqlite3_bind_text(stmt, 3, skill.c_str(), -1, SQLITE_TRANSIENT);
    bool found = (sqlite3_step(stmt) == SQLITE_ROW);
    sqlite3_finalize(stmt);
    return found;
}