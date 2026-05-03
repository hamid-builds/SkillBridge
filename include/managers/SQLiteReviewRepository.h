#ifndef SKILLBRIDGE_SQLITEREVIEWREPOSITORY_H
#define SKILLBRIDGE_SQLITEREVIEWREPOSITORY_H

#include "IReviewRepository.h"
#include <sqlite3.h>

class SQLiteReviewRepository : public IReviewRepository {
public:
    explicit SQLiteReviewRepository(sqlite3* db);
    ~SQLiteReviewRepository() override = default;

    SQLiteReviewRepository(const SQLiteReviewRepository&) = delete;
    SQLiteReviewRepository& operator=(const SQLiteReviewRepository&) = delete;

    void             saveReview(Review& review)               override;
    Review           findByID(int reviewID)             const override;
    DataList<Review> findByTarget(int targetUserID)     const override;
    DataList<Review> findByOrder(int orderID)           const override;
    DataList<Review> findAll()                          const override;
    bool             deleteReview(int reviewID)               override;
    double           averageRating(int targetUserID)    const override;
    bool             existsForOrder(int orderID)        const override;

private:
    sqlite3* db_;
    static Review buildFromRow(sqlite3_stmt* stmt);
};

#endif
