#include "../../include/managers/SQLiteReviewRepository.h"
#include "../../include/core/Exceptions.h"
#include <string>

SQLiteReviewRepository::SQLiteReviewRepository(sqlite3* db) : db_(db) {
    if (!db_)
        throw DatabaseException("SQLiteReviewRepository: null db handle");
}

Review SQLiteReviewRepository::buildFromRow(sqlite3_stmt* stmt) {
    Review r;
    r.setReviewID(sqlite3_column_int(stmt, 0));
    r.setOrderID(sqlite3_column_int(stmt, 1));
    r.setReviewerID(sqlite3_column_int(stmt, 2));
    r.setTargetUserID(sqlite3_column_int(stmt, 3));
    r.setRating(sqlite3_column_int(stmt, 4));
    r.setSentimentScore(sqlite3_column_double(stmt, 5));

    auto text = [&](int col) -> std::string {
        const unsigned char* t = sqlite3_column_text(stmt, col);
        return t ? reinterpret_cast<const char*>(t) : "";
        };
    r.setComment(text(6));
    r.setCreatedAt(text(7));
    return r;
}

void SQLiteReviewRepository::saveReview(Review& review) {
    if (existsForOrder(review.getOrderID()))
        throw DuplicateEntryException(
            "a review already exists for order #" +
            std::to_string(review.getOrderID()));

    const char* sql =
        "INSERT INTO reviews "
        "(orderID, reviewerID, targetUserID, rating, sentimentScore, comment, createdAt) "
        "VALUES (?, ?, ?, ?, ?, ?, ?);";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK)
        throw DatabaseException(sqlite3_errmsg(db_));

    sqlite3_bind_int(stmt, 1, review.getOrderID());
    sqlite3_bind_int(stmt, 2, review.getReviewerID());
    sqlite3_bind_int(stmt, 3, review.getTargetUserID());
    sqlite3_bind_int(stmt, 4, review.getRating());
    sqlite3_bind_double(stmt, 5, review.getSentimentScore());
    sqlite3_bind_text(stmt, 6, review.getComment().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 7, review.getCreatedAt().c_str(), -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE)
        throw DatabaseException(sqlite3_errmsg(db_));

    review.setReviewID(static_cast<int>(sqlite3_last_insert_rowid(db_)));
}

Review SQLiteReviewRepository::findByID(int reviewID) const {
    const char* sql =
        "SELECT reviewID, orderID, reviewerID, targetUserID, rating, "
        "sentimentScore, comment, createdAt FROM reviews WHERE reviewID = ?;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK)
        throw DatabaseException(sqlite3_errmsg(db_));

    sqlite3_bind_int(stmt, 1, reviewID);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        Review r = buildFromRow(stmt);
        sqlite3_finalize(stmt);
        return r;
    }
    sqlite3_finalize(stmt);
    throw ReviewNotFoundException("reviewID=" + std::to_string(reviewID));
}

DataList<Review> SQLiteReviewRepository::findByTarget(int targetUserID) const {
    const char* sql =
        "SELECT reviewID, orderID, reviewerID, targetUserID, rating, "
        "sentimentScore, comment, createdAt "
        "FROM reviews WHERE targetUserID = ? ORDER BY createdAt DESC;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK)
        throw DatabaseException(sqlite3_errmsg(db_));

    sqlite3_bind_int(stmt, 1, targetUserID);
    DataList<Review> results;
    while (sqlite3_step(stmt) == SQLITE_ROW)
        results.add(buildFromRow(stmt));
    sqlite3_finalize(stmt);
    return results;
}

DataList<Review> SQLiteReviewRepository::findByOrder(int orderID) const {
    const char* sql =
        "SELECT reviewID, orderID, reviewerID, targetUserID, rating, "
        "sentimentScore, comment, createdAt FROM reviews WHERE orderID = ?;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK)
        throw DatabaseException(sqlite3_errmsg(db_));

    sqlite3_bind_int(stmt, 1, orderID);
    DataList<Review> results;
    while (sqlite3_step(stmt) == SQLITE_ROW)
        results.add(buildFromRow(stmt));
    sqlite3_finalize(stmt);
    return results;
}

DataList<Review> SQLiteReviewRepository::findAll() const {
    const char* sql =
        "SELECT reviewID, orderID, reviewerID, targetUserID, rating, "
        "sentimentScore, comment, createdAt FROM reviews ORDER BY createdAt DESC;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK)
        throw DatabaseException(sqlite3_errmsg(db_));

    DataList<Review> results;
    while (sqlite3_step(stmt) == SQLITE_ROW)
        results.add(buildFromRow(stmt));
    sqlite3_finalize(stmt);
    return results;
}

bool SQLiteReviewRepository::deleteReview(int reviewID) {
    const char* sql = "DELETE FROM reviews WHERE reviewID = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK)
        throw DatabaseException(sqlite3_errmsg(db_));
    sqlite3_bind_int(stmt, 1, reviewID);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return sqlite3_changes(db_) > 0;
}

double SQLiteReviewRepository::averageRating(int targetUserID) const {
    const char* sql =
        "SELECT AVG(CAST(rating AS REAL)) FROM reviews WHERE targetUserID = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK)
        throw DatabaseException(sqlite3_errmsg(db_));
    sqlite3_bind_int(stmt, 1, targetUserID);
    double avg = 0.0;
    if (sqlite3_step(stmt) == SQLITE_ROW &&
        sqlite3_column_type(stmt, 0) != SQLITE_NULL)
        avg = sqlite3_column_double(stmt, 0);
    sqlite3_finalize(stmt);
    return avg;
}

bool SQLiteReviewRepository::existsForOrder(int orderID) const {
    const char* sql = "SELECT 1 FROM reviews WHERE orderID = ? LIMIT 1;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK)
        throw DatabaseException(sqlite3_errmsg(db_));
    sqlite3_bind_int(stmt, 1, orderID);
    bool found = (sqlite3_step(stmt) == SQLITE_ROW);
    sqlite3_finalize(stmt);
    return found;
}