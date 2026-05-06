#ifndef SKILLBRIDGE_IGIGREPOSITORY_H
#define SKILLBRIDGE_IGIGREPOSITORY_H

#include "core/Gig.h"
#include "core/GigBrowseFilter.h"
#include "utils/DataList.h"


class IGigRepository 
{
public:
    virtual ~IGigRepository() = default;
   
    virtual void saveGig(Gig& gig) = 0;

    virtual void updateGig(const Gig& gig) = 0;
   
    virtual void deactivateGig(int gigID) = 0;

    virtual void setGigActive(int gigID, bool active) = 0;
    
    virtual bool deleteGig(int gigID) = 0;

    virtual Gig findGigByID(int gigID) const = 0;
    
    virtual DataList<Gig> findGigsByOwner(int ownerID) const = 0;

    virtual DataList<Gig> findAllActiveGigs() const = 0;
    
    virtual DataList<Gig> findAllGigs() const = 0;

    virtual DataList<Gig> findActiveGigsFiltered(const GigBrowseFilter& filter, GigSortOrder sort) const = 0;
};

#endif