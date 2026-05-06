#ifndef SKILLBRIDGE_API_SERIALIZERS_H
#define SKILLBRIDGE_API_SERIALIZERS_H

#include "json.hpp"
#include <string>
class User;
class Gig;
class Freelancer;
class Order;
class Review;
class Endorsement;
struct DecodedMessage;
namespace sb 
{
    namespace api 
    {
        nlohmann::json toJson(const User& u);
        nlohmann::json toJson(const Gig& g);
        nlohmann::json toJson(const Order& o);
        nlohmann::json toJson(const Review& r);
        nlohmann::json toJson(const Endorsement& e);

        nlohmann::json toBrowseCardJson(const Gig& g, const std::string& freelancerName, double freelancerAvgRating);

        nlohmann::json toGigDetailJson(const Gig& g);

        nlohmann::json toFreelancerDetailJson(const Freelancer& f, int reviewCount);

        nlohmann::json toReviewCardJson(const Review& r, const std::string& reviewerName);

        nlohmann::json toUserPublicProfileJson(const User& u, int reviewCount);
        
        nlohmann::json toOrderCardJson(const Order& o, const std::string& gigTitle, const std::string& otherPartyName, int otherPartyID);

        nlohmann::json toMessageJson(const DecodedMessage& dm, const std::string& senderName);

        nlohmann::json toEndorsementReceivedJson(const Endorsement& e, const std::string& fromUserName);

        nlohmann::json toEndorsementGivenJson(const Endorsement& e, const std::string& toUserName);

        nlohmann::json toRankedUserJson(int userID, const std::string& userName, double score, int rank);

        nlohmann::json toTrustedUserJson(int userID, const std::string& userName, int hopCount);

        nlohmann::json toAdminGigJson(const Gig& g, const std::string& ownerName);

    }
} 
#endif