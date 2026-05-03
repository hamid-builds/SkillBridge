#ifndef SKILLBRIDGE_REVIEWMANAGER_H
#define SKILLBRIDGE_REVIEWMANAGER_H

#include "IReviewRepository.h"
#include "IOrderRepository.h"
#include "IUserRepository.h"
#include "../strategies/ISentimentAnalyzer.h"
#include "../core/Review.h"
#include "../utils/DataList.h"


class ReviewManager {
public:
    ReviewManager(IReviewRepository& reviewRepo,
        IOrderRepository& orderRepo,
        IUserRepository& userRepo,
        ISentimentAnalyzer& analyzer);

    ~ReviewManager() = default;

    ReviewManager(const ReviewManager&) = delete;
    ReviewManager& operator=(const ReviewManager&) = delete;

    Review submitReview(int currentUserID,
        int orderID,
        int rating,
        const std::string& comment);

   DataList<Review> getReviewsForFreelancer(int currentUserID,
        int targetUserID) const;

    double getAverageRating(int targetUserID) const;
    bool adminDeleteReview(int currentUserID, int reviewID);

private:
    IReviewRepository& reviewRepo_;
    IOrderRepository& orderRepo_;
    IUserRepository& userRepo_;
    ISentimentAnalyzer& analyzer_;

    static std::string currentTimestamp();
    void requireAdmin(int currentUserID) const;
};

#endif