#ifndef SKILLBRIDGE_SKILLGRAPHMANAGER_H
#define SKILLBRIDGE_SKILLGRAPHMANAGER_H

#include "IEndorsementRepository.h"
#include "IUserRepository.h"
#include "../utils/SkillGraph.h"
#include "../utils/PageRankCalculator.h"
#include "../utils/GraphTraverser.h"




class SkillGraphManager {
public:
    SkillGraphManager(IEndorsementRepository& endorsementRepo,
        IUserRepository& userRepo);

    ~SkillGraphManager() = default;

    SkillGraphManager(const SkillGraphManager&) = delete;
    SkillGraphManager& operator=(const SkillGraphManager&) = delete;

    
    Endorsement endorseUser(int currentUserID,
        int targetUserID,
        const std::string& skill,
        double weight = 1.0);

    
    bool removeEndorsement(int currentUserID, int endorsementID);

   
    DataList<RankedUser> getTopFreelancers(int currentUserID, int topN = 10) const;

   
    DataList<TrustedUser> getTrustedNear(int currentUserID, int maxHops = 2) const;

   
    DataList<Endorsement> getEndorsementsFor(int currentUserID,
        int targetUserID) const;

    void clearGraph();

private:
    IEndorsementRepository& endorsementRepo_;
    IUserRepository& userRepo_;
    SkillGraph              graph_;
    PageRankCalculator      pageRank_;
    GraphTraverser          traverser_;

    static std::string currentTimestamp();
    void requireAdmin(int currentUserID) const;
};

#endif