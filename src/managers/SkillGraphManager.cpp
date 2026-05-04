#include "../../include/managers/SkillGraphManager.h"
#include "../../include/core/Exceptions.h"
#include "../../include/core/User.h"
#include <ctime>
#include <cstdio>

SkillGraphManager::SkillGraphManager(IEndorsementRepository& endorsementRepo,
    IUserRepository& userRepo)
    : endorsementRepo_(endorsementRepo),
    userRepo_(userRepo),
    graph_(),
    pageRank_(),
    traverser_()
{
    DataList<Endorsement> all = endorsementRepo_.findAll();
    for (int i = 0; i < all.size(); ++i)
        graph_.addEdge(all[i]);
}

Endorsement SkillGraphManager::endorseUser(int currentUserID,
    int targetUserID,
    const std::string& skill,
    double weight)
{
    if (currentUserID <= 0)
        throw UnauthorizedException("must be logged in to endorse");

    User* target = userRepo_.findUserByID(targetUserID);
    if (!target)
        throw DatabaseException(
            "target user #" + std::to_string(targetUserID) + " does not exist");
    delete target;

    Endorsement e(currentUserID, targetUserID, skill, weight);
    e.setTimestamp(currentTimestamp());

    endorsementRepo_.saveEndorsement(e);
    graph_.addEdge(e);

    return e;
}

bool SkillGraphManager::removeEndorsement(int currentUserID, int endorsementID) {
    Endorsement e = endorsementRepo_.findByID(endorsementID);

    if (e.getFromUserID() != currentUserID)
        requireAdmin(currentUserID);

    bool deleted = endorsementRepo_.deleteEndorsement(endorsementID);
    if (deleted)
        graph_.removeEdge(endorsementID);

    return deleted;
}

DataList<RankedUser> SkillGraphManager::getTopFreelancers(int currentUserID,
    int topN) const
{
    if (currentUserID <= 0)
        throw UnauthorizedException("must be logged in to view rankings");

    DataList<RankedUser> all = pageRank_.calculate(graph_);

    if (topN <= 0 || topN >= all.size()) return all;

    DataList<RankedUser> top;
    for (int i = 0; i < topN; ++i)
        top.add(all[i]);
    return top;
}

DataList<TrustedUser> SkillGraphManager::getTrustedNear(int currentUserID,
    int maxHops) const
{
    if (currentUserID <= 0)
        throw UnauthorizedException("must be logged in to view trusted users");

    GraphTraverser t(maxHops);
    return t.findTrusted(graph_, currentUserID);
}

DataList<Endorsement> SkillGraphManager::getEndorsementsFor(int currentUserID,
    int targetUserID) const
{
    if (currentUserID <= 0)
        throw UnauthorizedException("must be logged in to view endorsements");

    return endorsementRepo_.findByTo(targetUserID);
}

void SkillGraphManager::clearGraph() {
    graph_.clear();
}

std::string SkillGraphManager::currentTimestamp() {
    std::time_t now = std::time(nullptr);
    std::tm lt{};
#ifdef _WIN32
    localtime_s(&lt, &now);
#else
    localtime_r(&now, &lt);
#endif
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d",
        lt.tm_year + 1900, lt.tm_mon + 1, lt.tm_mday,
        lt.tm_hour, lt.tm_min, lt.tm_sec);
    return std::string(buf);
}

void SkillGraphManager::requireAdmin(int currentUserID) const {
    User* user = userRepo_.findUserByID(currentUserID);
    if (!user)
        throw UnauthorizedException("user not found");
    UserRole role = user->getRole();
    delete user;
    if (role != UserRole::ADMIN)
        throw UnauthorizedException("admin access required");
}