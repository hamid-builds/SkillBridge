

#include "managers/ReviewManager.h"
#include "core/Exceptions.h"
#include "core/User.h"
#include "core/Freelancer.h"
#include <ctime>
#include <cstdio>
#include <string>

ReviewManager::ReviewManager(IReviewRepository& reviewRepo,
    IOrderRepository& orderRepo,
    IUserRepository& userRepo,
    ISentimentAnalyzer& analyzer)
    : reviewRepo_(reviewRepo),
    orderRepo_(orderRepo),
    userRepo_(userRepo),
    analyzer_(analyzer)
{
}

Review ReviewManager::submitReview(int currentUserID,
    int orderID,
    int rating,
    const std::string& comment)
{
   
    Order order = orderRepo_.findOrderByID(orderID);

    if (order.getStatus() != OrderStatus::COMPLETED)
        throw ValidationException(
            "can only review a completed order (order #" +
            std::to_string(orderID) + " is not completed)");

    
    if (currentUserID != order.getBuyerID())
        throw UnauthorizedException(
            "only the buyer of an order may leave a review");

   
    if (reviewRepo_.existsForOrder(orderID))
        throw DuplicateEntryException(
            "a review already exists for order #" +
            std::to_string(orderID));

    Review review(orderID, currentUserID, order.getSellerID(),
        rating, comment);

    review.setSentimentScore(analyzer_.analyze(comment));

    review.setCreatedAt(currentTimestamp());

    reviewRepo_.saveReview(review);

    double newAvg = reviewRepo_.averageRating(order.getSellerID());
    User* seller = userRepo_.findUserByID(order.getSellerID());
    if (seller) {
        Freelancer* f = dynamic_cast<Freelancer*>(seller);
        if (f) {
            f->updateAvgRating(newAvg);
            userRepo_.updateUser(f);
        }
        delete seller;
    }

    return review;
}

DataList<Review> ReviewManager::getReviewsForFreelancer(int currentUserID,
    int targetUserID) const
{
    if (currentUserID <= 0)
        throw UnauthorizedException("must be logged in to view reviews");
    return reviewRepo_.findByTarget(targetUserID);
}

double ReviewManager::getAverageRating(int targetUserID) const {
    return reviewRepo_.averageRating(targetUserID);
}

bool ReviewManager::adminDeleteReview(int currentUserID, int reviewID) {
    requireAdmin(currentUserID);
    reviewRepo_.findByID(reviewID); 

    return reviewRepo_.deleteReview(reviewID);
}

std::string ReviewManager::currentTimestamp() {
    std::time_t now = std::time(nullptr);
    std::tm lt{};
    localtime_s(&lt, &now);
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d",
        lt.tm_year + 1900, lt.tm_mon + 1, lt.tm_mday,
        lt.tm_hour, lt.tm_min, lt.tm_sec);
    return std::string(buf);
}

void ReviewManager::requireAdmin(int currentUserID) const {
    User* user = userRepo_.findUserByID(currentUserID);
    if (!user)
        throw UnauthorizedException("user not found");
    UserRole role = user->getRole();
    delete user;
    if (role != UserRole::ADMIN)
        throw UnauthorizedException("admin access required");
}