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

namespace sb 
{
    namespace api 
    {
        nlohmann::json toJson(const User& u) 
        {
            nlohmann::json j;
            j["userID"] = u.getUserID();
            j["name"] = u.getName();
            j["email"] = u.getEmail();
            j["role"] = roleToString(u.getRole());
            j["balance"] = u.getBalance();
            j["createdAt"] = u.getCreatedAt();

            const Freelancer* f = dynamic_cast<const Freelancer*>(&u);
            if (f != nullptr) 
            {
                j["portfolio"] = f->getPortfolio();
                j["skills"] = f->getSkills();
                j["avgRating"] = f->getAvgRating();
            }

            return j;
        }

        nlohmann::json toJson(const Gig& g) 
        {
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

        nlohmann::json toJson(const Order& o) 
        {
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

        nlohmann::json toJson(const Review& r) 
        {
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

        nlohmann::json toJson(const Endorsement& e) 
        {
            nlohmann::json j;
            j["endorsementID"] = e.getEndorsementID();
            j["fromUserID"] = e.getFromUserID();
            j["toUserID"] = e.getToUserID();
            j["skill"] = e.getSkill();
            j["weight"] = e.getWeight();
            j["timestamp"] = e.getTimestamp();
            return j;
        }

    }
}