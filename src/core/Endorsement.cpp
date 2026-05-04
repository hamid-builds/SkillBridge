#include "../../include/core/Endorsement.h"
#include "../../include/core/Exceptions.h"
#include <cctype>
#include <ostream>

static bool isBlank(const std::string& s) {
    for (char c : s)
        if (!std::isspace(static_cast<unsigned char>(c))) return false;
    return true;
}

Endorsement::Endorsement(int fromUserID, int toUserID,
    const std::string& skill, double weight)
    : fromUserID_(fromUserID),
    toUserID_(toUserID),
    skill_(skill),
    weight_(weight)
{
    if (fromUserID <= 0)
        throw ValidationException("fromUserID must be positive");
    if (toUserID <= 0)
        throw ValidationException("toUserID must be positive");
    if (fromUserID == toUserID)
        throw ValidationException("a user cannot endorse themselves");
    if (isBlank(skill))
        throw ValidationException("skill must not be empty");
    if (weight <= 0.0 || weight > 10.0)
        throw ValidationException("weight must be in (0.0, 10.0]");
}

std::ostream& operator<<(std::ostream& os, const Endorsement& e) {
    os << "[Endorsement #" << e.endorsementID_
        << " | " << e.fromUserID_ << " -> " << e.toUserID_
        << " | Skill: " << e.skill_
        << " | Weight: " << e.weight_
        << " | " << e.timestamp_ << "]";
    return os;
}