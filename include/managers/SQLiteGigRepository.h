#ifndef SKILLBRIDGE_SQLITEGIGREPOSITORY_H
#define SKILLBRIDGE_SQLITEGIGREPOSITORY_H

#include "IGigRepository.h"


struct sqlite3_stmt;



class SQLiteGigRepository : public IGigRepository {
public:
    SQLiteGigRepository() = default;
    ~SQLiteGigRepository() override = default;

    // Write operations
    void saveGig(Gig& gig) override;
    void updateGig(const Gig& gig) override;
    void deactivateGig(int gigID) override;
    void setGigActive(int gigID, bool active) override;
    bool deleteGig(int gigID) override;

    // Read operations
    Gig           findGigByID(int gigID) const override;
    DataList<Gig> findGigsByOwner(int ownerID) const override;
    DataList<Gig> findAllActiveGigs() const override;
    DataList<Gig> findAllGigs() const override;
    DataList<Gig> findActiveGigsFiltered(const GigBrowseFilter& filter, GigSortOrder sort) const override;

private:
   
    Gig buildGigFromRow(sqlite3_stmt* stmt) const;

   
    void throwPrepareError(const std::string& sql) const;
};

#endif