#ifndef SKILLBRIDGE_SQLITEENDORSEMENTREPOSITORY_H
#define SKILLBRIDGE_SQLITEENDORSEMENTREPOSITORY_H

#include "IEndorsementRepository.h"
#include <sqlite3.h>




class SQLiteEndorsementRepository : public IEndorsementRepository {
public:
    explicit SQLiteEndorsementRepository(sqlite3* db);
    ~SQLiteEndorsementRepository() override = default;

    SQLiteEndorsementRepository(const SQLiteEndorsementRepository&) = delete;
    SQLiteEndorsementRepository& operator=(const SQLiteEndorsementRepository&) = delete;

    void                  saveEndorsement(Endorsement& endorsement)        override;
    Endorsement           findByID(int endorsementID)               const  override;
    DataList<Endorsement> findByFrom(int fromUserID)                const  override;
    DataList<Endorsement> findByTo(int toUserID)                    const  override;
    DataList<Endorsement> findAll()                                 const  override;
    bool                  deleteEndorsement(int endorsementID)             override;
    bool                  exists(int fromUserID, int toUserID,
        const std::string& skill)          const  override;

private:
    sqlite3* db_;
    static Endorsement buildFromRow(sqlite3_stmt* stmt);
};

#endif