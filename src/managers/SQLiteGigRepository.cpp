#include "managers/SQLiteGigRepository.h"
#include "managers/DatabaseManager.h"
#include "core/Exceptions.h"
#include "sqlite3.h"

#include <string>

using namespace std;




void SQLiteGigRepository::throwPrepareError(const string& sql) const {
    sqlite3* db = DatabaseManager::getInstance().getConnection();
    string msg = "Failed to prepare statement. SQL: " + sql +
        " | Error: " + sqlite3_errmsg(db);
    throw DatabaseException(msg);
}

Gig SQLiteGigRepository::buildGigFromRow(sqlite3_stmt* stmt) const {
    
    int gigID = sqlite3_column_int(stmt, 0);
    int ownerID = sqlite3_column_int(stmt, 1);
    const char* title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
    const char* desc = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
    double price = sqlite3_column_double(stmt, 4);
    const char* cat = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
    int active = sqlite3_column_int(stmt, 6);
    const char* ts = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));

    
    Gig g;
    g.setGigID(gigID);
    g.setOwnerID(ownerID);
    g.setTitle(title ? title : "");
    g.setDescription(desc ? desc : "");
    g.setPrice(price);
    g.setCategory(gigCategoryFromString(cat ? cat : "OTHER"));
    g.setIsActive(active != 0);
    g.setCreatedAt(ts ? ts : "");
    return g;
}



void SQLiteGigRepository::saveGig(Gig& gig) {
    sqlite3* db = DatabaseManager::getInstance().getConnection();

    const string sql =
        "INSERT INTO gigs (ownerID, title, description, price, category, isActive) "
        "VALUES (?, ?, ?, ?, ?, ?);";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        throwPrepareError(sql);
    }

    const string categoryStr = gigCategoryToString(gig.getCategory());

    sqlite3_bind_int(stmt, 1, gig.getOwnerID());
    sqlite3_bind_text(stmt, 2, gig.getTitle().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, gig.getDescription().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 4, gig.getPrice());
    sqlite3_bind_text(stmt, 5, categoryStr.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 6, gig.getIsActive() ? 1 : 0);

    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
       
        string err = sqlite3_errmsg(db);
        int extended = sqlite3_extended_errcode(db);
        sqlite3_finalize(stmt);

       
      
        if (rc == SQLITE_CONSTRAINT) {
            if (extended == SQLITE_CONSTRAINT_UNIQUE) {
                throw DuplicateEntryException(
                    "Gig unique constraint: " + err);
            }
            
            throw ValidationException("Gig constraint failed: " + err);
        }
        throw DatabaseException("Failed to insert gig: " + err);
    }

    int newID = static_cast<int>(sqlite3_last_insert_rowid(db));
    sqlite3_finalize(stmt);

    gig.setGigID(newID);

  
    try {
        Gig reloaded = findGigByID(newID);
        gig.setCreatedAt(reloaded.getCreatedAt());
    }
    catch (const exception&) {
       
    }
}



void SQLiteGigRepository::updateGig(const Gig& gig) {
    sqlite3* db = DatabaseManager::getInstance().getConnection();

    const string sql =
        "UPDATE gigs SET ownerID=?, title=?, description=?, price=?, "
        "category=?, isActive=? WHERE gigID=?;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        throwPrepareError(sql);
    }

    const string categoryStr = gigCategoryToString(gig.getCategory());

    sqlite3_bind_int(stmt, 1, gig.getOwnerID());
    sqlite3_bind_text(stmt, 2, gig.getTitle().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, gig.getDescription().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 4, gig.getPrice());
    sqlite3_bind_text(stmt, 5, categoryStr.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 6, gig.getIsActive() ? 1 : 0);
    sqlite3_bind_int(stmt, 7, gig.getGigID());

    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        string err = sqlite3_errmsg(db);
        sqlite3_finalize(stmt);
        throw DatabaseException("Failed to update gig: " + err);
    }

    int changed = sqlite3_changes(db);
    sqlite3_finalize(stmt);

    if (changed == 0) {
        throw GigNotFoundException(
            "No gig with gigID=" + to_string(gig.getGigID()));
    }
}



void SQLiteGigRepository::deactivateGig(int gigID) {
    sqlite3* db = DatabaseManager::getInstance().getConnection();

    const string sql = "UPDATE gigs SET isActive=0 WHERE gigID=?;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        throwPrepareError(sql);
    }

    sqlite3_bind_int(stmt, 1, gigID);

    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        string err = sqlite3_errmsg(db);
        sqlite3_finalize(stmt);
        throw DatabaseException("Failed to deactivate gig: " + err);
    }

    int changed = sqlite3_changes(db);
    sqlite3_finalize(stmt);

    if (changed == 0) {
        throw GigNotFoundException("No gig with gigID=" + to_string(gigID));
    }
}



bool SQLiteGigRepository::deleteGig(int gigID) {
    sqlite3* db = DatabaseManager::getInstance().getConnection();

    const string sql = "DELETE FROM gigs WHERE gigID=?;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        throwPrepareError(sql);
    }

    sqlite3_bind_int(stmt, 1, gigID);

    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        string err = sqlite3_errmsg(db);
        sqlite3_finalize(stmt);
        throw DatabaseException("Failed to delete gig: " + err);
    }

    int changed = sqlite3_changes(db);
    sqlite3_finalize(stmt);

    return changed > 0;
}



Gig SQLiteGigRepository::findGigByID(int gigID) const {
    sqlite3* db = DatabaseManager::getInstance().getConnection();

    const string sql =
        "SELECT gigID, ownerID, title, description, price, category, "
        "       isActive, createdAt "
        "FROM gigs WHERE gigID=?;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        throwPrepareError(sql);
    }

    sqlite3_bind_int(stmt, 1, gigID);

    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        Gig g = buildGigFromRow(stmt);
        sqlite3_finalize(stmt);
        return g;
    }

    sqlite3_finalize(stmt);

    if (rc == SQLITE_DONE) {
        throw GigNotFoundException("No gig with gigID=" + to_string(gigID));
    }

    throw DatabaseException(
        "Failed to query gig by ID: " + string(sqlite3_errmsg(db)));
}



DataList<Gig> SQLiteGigRepository::findGigsByOwner(int ownerID) const {
    sqlite3* db = DatabaseManager::getInstance().getConnection();

    const string sql =
        "SELECT gigID, ownerID, title, description, price, category, "
        "       isActive, createdAt "
        "FROM gigs WHERE ownerID=? ORDER BY gigID ASC;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        throwPrepareError(sql);
    }

    sqlite3_bind_int(stmt, 1, ownerID);

    DataList<Gig> result;
    int rc;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        result.add(buildGigFromRow(stmt));
    }

    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        throw DatabaseException(
            "Failed to query gigs by owner: " + string(sqlite3_errmsg(db)));
    }

    return result;
}



DataList<Gig> SQLiteGigRepository::findAllActiveGigs() const {
    sqlite3* db = DatabaseManager::getInstance().getConnection();

    const string sql =
        "SELECT gigID, ownerID, title, description, price, category, "
        "       isActive, createdAt "
        "FROM gigs WHERE isActive=1 ORDER BY gigID ASC;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        throwPrepareError(sql);
    }

    DataList<Gig> result;
    int rc;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        result.add(buildGigFromRow(stmt));
    }

    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        throw DatabaseException(
            "Failed to query active gigs: " + string(sqlite3_errmsg(db)));
    }

    return result;
}



DataList<Gig> SQLiteGigRepository::findAllGigs() const {
    sqlite3* db = DatabaseManager::getInstance().getConnection();

    const string sql =
        "SELECT gigID, ownerID, title, description, price, category, "
        "       isActive, createdAt "
        "FROM gigs ORDER BY gigID ASC;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        throwPrepareError(sql);
    }

    DataList<Gig> result;
    int rc;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        result.add(buildGigFromRow(stmt));
    }

    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        throw DatabaseException(
            "Failed to query all gigs: " + string(sqlite3_errmsg(db)));
    }

    return result;
}

DataList<Gig> SQLiteGigRepository::findActiveGigsFiltered(
    const GigBrowseFilter& filter,
    GigSortOrder sort
) const {
    sqlite3* db = DatabaseManager::getInstance().getConnection();

    string sql =
        "SELECT gigID, ownerID, title, description, price, category, "
        "       isActive, createdAt "
        "FROM gigs WHERE isActive=1";

    if (filter.hasCategory) {
        sql += " AND category=?";
    }

    switch (sort) {
    case GigSortOrder::NEWEST_FIRST:
        sql += " ORDER BY createdAt DESC, gigID DESC";
        break;
    case GigSortOrder::PRICE_ASC:
        sql += " ORDER BY price ASC, gigID ASC";
        break;
    case GigSortOrder::PRICE_DESC:
        sql += " ORDER BY price DESC, gigID ASC";
        break;
    default:
        sql += " ORDER BY createdAt DESC, gigID DESC";
        break;
    }
    sql += ";";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        throwPrepareError(sql);
    }

    if (filter.hasCategory) {
        const string categoryStr = gigCategoryToString(filter.category);
        sqlite3_bind_text(stmt, 1, categoryStr.c_str(), -1, SQLITE_TRANSIENT);
    }

    DataList<Gig> result;
    int rc;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        result.add(buildGigFromRow(stmt));
    }

    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        throw DatabaseException(
            "Failed to query filtered active gigs: " +
            string(sqlite3_errmsg(db)));
    }

    return result;
}
-

void SQLiteGigRepository::setGigActive(int gigID, bool active) {
    sqlite3* db = DatabaseManager::getInstance().getConnection();

    const string sql = "UPDATE gigs SET isActive=? WHERE gigID=?;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        throwPrepareError(sql);
    }

    sqlite3_bind_int(stmt, 1, active ? 1 : 0);
    sqlite3_bind_int(stmt, 2, gigID);

    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        string err = sqlite3_errmsg(db);
        sqlite3_finalize(stmt);
        throw DatabaseException("Failed to set gig active flag: " + err);
    }

    int changed = sqlite3_changes(db);
    sqlite3_finalize(stmt);

    if (changed == 0) {
        throw GigNotFoundException("No gig with gigID=" + to_string(gigID));
    }
}