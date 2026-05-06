#include "api/Serializers.h"

#include "core/User.h"
#include "core/Freelancer.h"
#include "core/UserRole.h"
#include "core/Gig.h"
#include "core/GigCategory.h"
#include "core/Order.h"
#include "core/OrderStatus.h"
#include "core/Review.h"
#include "core/Endorsement.h"
#include "managers/MessageManager.h"

namespace sb {
    namespace api {

        nlohmann::json toJson(const User& u) {
            nlohmann::json j;
            j["userID"] = u.getUserID();
            j["name"] = u.getName();
            j["email"] = u.getEmail();
            j["role"] = roleToString(u.getRole());
            j["balance"] = u.getBalance();
            j["createdAt"] = u.getCreatedAt();

            const Freelancer* f = dynamic_cast<const Freelancer*>(&u);
            if (f != nullptr) {
                j["portfolio"] = f->getPortfolio();
                j["skills"] = f->getSkills();
                j["avgRating"] = f->getAvgRating();
            }

            return j;
        }

        nlohmann::json toJson(const Gig& g) {
            nlohmann::json j;
            j["gigID"] = g.getGigID();
            j["ownerID"] = g.getOwnerID();
            j["title"] = g.getTitle();
            j["description"] = g.getDescription();
            j["price"] = g.getPrice();
            j["category"] = gigCategoryToString(g.getCategory());
            j["isActive"] = g.getIsActive();
            j["createdAt"] = g.getCreatedAt();
            return j;
        }

        nlohmann::json toJson(const Order& o) {
            nlohmann::json j;
            j["orderID"] = o.getOrderID();
            j["gigID"] = o.getGigID();
            j["buyerID"] = o.getBuyerID();
            j["sellerID"] = o.getSellerID();
            j["amount"] = o.getAmount();
            j["status"] = orderStatusToString(o.getStatus());
            j["placedAt"] = o.getPlacedAt();
            j["completedAt"] = o.getCompletedAt();
            j["deadline"] = o.getDeadline();
            return j;
        }

        nlohmann::json toJson(const Review& r) {
            nlohmann::json j;
            j["reviewID"] = r.getReviewID();
            j["orderID"] = r.getOrderID();
            j["reviewerID"] = r.getReviewerID();
            j["targetUserID"] = r.getTargetUserID();
            j["rating"] = r.getRating();
            j["sentimentScore"] = r.getSentimentScore();
            j["comment"] = r.getComment();
            j["createdAt"] = r.getCreatedAt();
            return j;
        }

        nlohmann::json toJson(const Endorsement& e) {
            nlohmann::json j;
            j["endorsementID"] = e.getEndorsementID();
            j["fromUserID"] = e.getFromUserID();
            j["toUserID"] = e.getToUserID();
            j["skill"] = e.getSkill();
            j["weight"] = e.getWeight();
            j["timestamp"] = e.getTimestamp();
            return j;
        }

        nlohmann::json toBrowseCardJson(
            const Gig& g,
            const std::string& freelancerName,
            double freelancerAvgRating
        ) {
            nlohmann::json j;
            j["gigID"] = g.getGigID();
            j["title"] = g.getTitle();

            const std::string& full = g.getDescription();
            if (full.size() <= 140) {
                j["shortDescription"] = full;
            }
            else {
                j["shortDescription"] = full.substr(0, 137) + "...";
            }

            j["price"] = g.getPrice();
            j["category"] = gigCategoryToString(g.getCategory());
            j["freelancerName"] = freelancerName;
            j["freelancerAvgRating"] = freelancerAvgRating;

            return j;
        }

        nlohmann::json toGigDetailJson(const Gig& g) {
            nlohmann::json j;
            j["gigID"] = g.getGigID();
            j["title"] = g.getTitle();
            j["description"] = g.getDescription();
            j["price"] = g.getPrice();
            j["category"] = gigCategoryToString(g.getCategory());
            j["isActive"] = g.getIsActive();
            j["createdAt"] = g.getCreatedAt();
            return j;
        }

        nlohmann::json toFreelancerDetailJson(
            const Freelancer& f,
            int reviewCount
        ) {
            nlohmann::json j;
            j["userID"] = f.getUserID();
            j["name"] = f.getName();
            j["portfolio"] = f.getPortfolio();
            j["skills"] = f.getSkills();
            j["avgRating"] = f.getAvgRating();
            j["reviewCount"] = reviewCount;
            return j;
        }

        nlohmann::json toReviewCardJson(
            const Review& r,
            const std::string& reviewerName
        ) {
            nlohmann::json j;
            j["reviewID"] = r.getReviewID();
            j["rating"] = r.getRating();
            j["comment"] = r.getComment();
            j["createdAt"] = r.getCreatedAt();
            j["reviewerName"] = reviewerName;
            return j;
        }

        nlohmann::json toUserPublicProfileJson(
            const User& u,
            int reviewCount
        ) {
            nlohmann::json j;
            j["userID"] = u.getUserID();
            j["name"] = u.getName();
            j["role"] = roleToString(u.getRole());
            j["createdAt"] = u.getCreatedAt();

            const Freelancer* f = dynamic_cast<const Freelancer*>(&u);
            if (f != nullptr) {
                j["portfolio"] = f->getPortfolio();
                j["skills"] = f->getSkills();
                j["avgRating"] = f->getAvgRating();
                j["reviewCount"] = reviewCount;
            }

            return j;
        }

        nlohmann::json toOrderCardJson(
            const Order& o,
            const std::string& gigTitle,
            const std::string& otherPartyName,
            int otherPartyID
        ) {
            nlohmann::json j;
            j["orderID"] = o.getOrderID();
            j["gigID"] = o.getGigID();
            j["gigTitle"] = gigTitle;
            j["amount"] = o.getAmount();
            j["status"] = orderStatusToString(o.getStatus());
            j["deadline"] = o.getDeadline();
            j["placedAt"] = o.getPlacedAt();
            j["completedAt"] = o.getCompletedAt();
            j["otherPartyName"] = otherPartyName;
            j["otherPartyID"] = otherPartyID;
            return j;
        }

        nlohmann::json toMessageJson(
            const DecodedMessage& dm,
            const std::string& senderName
        ) {
            nlohmann::json j;
            j["messageID"] = dm.messageID;
            j["senderID"] = dm.senderID;
            j["senderName"] = senderName;
            j["receiverID"] = dm.receiverID;
            j["text"] = dm.text;
            j["timestamp"] = dm.timestamp;
            j["isRead"] = dm.isRead;
            return j;
        }

        nlohmann::json toEndorsementReceivedJson(
            const Endorsement& e,
            const std::string& fromUserName
        ) {
            nlohmann::json j;
            j["fromUserID"] = e.getFromUserID();
            j["fromUserName"] = fromUserName;
            j["skill"] = e.getSkill();
            j["weight"] = e.getWeight();
            j["timestamp"] = e.getTimestamp();
            return j;
        }

        nlohmann::json toEndorsementGivenJson(
            const Endorsement& e,
            const std::string& toUserName
        ) {
            nlohmann::json j;
            j["endorsementID"] = e.getEndorsementID();
            j["toUserID"] = e.getToUserID();
            j["toUserName"] = toUserName;
            j["skill"] = e.getSkill();
            j["weight"] = e.getWeight();
            j["timestamp"] = e.getTimestamp();
            return j;
        }

        nlohmann::json toRankedUserJson(
            int userID,
            const std::string& userName,
            double score,
            int rank
        ) {
            nlohmann::json j;
            j["userID"] = userID;
            j["name"] = userName;
            j["score"] = score;
            j["rank"] = rank;
            return j;
        }

        nlohmann::json toTrustedUserJson(
            int userID,
            const std::string& userName,
            int hopCount
        ) {
            nlohmann::json j;
            j["userID"] = userID;
            j["name"] = userName;
            j["hopCount"] = hopCount;
            return j;
        }

        nlohmann::json toAdminGigJson(
            const Gig& g,
            const std::string& ownerName
        ) {
            nlohmann::json j;
            j["gigID"] = g.getGigID();
            j["title"] = g.getTitle();
            j["price"] = g.getPrice();
            j["category"] = gigCategoryToString(g.getCategory());
            j["isActive"] = g.getIsActive();
            j["createdAt"] = g.getCreatedAt();
            j["ownerID"] = g.getOwnerID();
            j["ownerName"] = ownerName;
            return j;
        }

    }
} 