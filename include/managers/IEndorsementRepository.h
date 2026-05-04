#ifndef SKILLBRIDGE_IENDORSEMENTREPOSITORY_H
#define SKILLBRIDGE_IENDORSEMENTREPOSITORY_H

#include "../core/Endorsement.h"
#include "../utils/DataList.h"
#include <string>



class IEndorsementRepository {
public:
    virtual ~IEndorsementRepository() = default;

    virtual void                  saveEndorsement(Endorsement& endorsement) = 0;
    virtual Endorsement           findByID(int endorsementID)              const = 0;
    virtual DataList<Endorsement> findByFrom(int fromUserID)               const = 0;
    virtual DataList<Endorsement> findByTo(int toUserID)                   const = 0;
    virtual DataList<Endorsement> findAll()                                const = 0;
    virtual bool                  deleteEndorsement(int endorsementID) = 0;
    virtual bool                  exists(int fromUserID, int toUserID,
        const std::string& skill)         const = 0;
};

#endif