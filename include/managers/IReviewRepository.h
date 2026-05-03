#ifndef SKILLBRIDGE_IREVIEWREPOSITORY_H
#define SKILLBRIDGE_IREVIEWREPOSITORY_H

#include "../core/Review.h"
#include "../utils/DataList.h"
#include <string>

class IReviewRepository {
public:
    virtual ~IReviewRepository() = default;

    virtual void             saveReview(Review& review) = 0;
    virtual Review           findByID(int reviewID)             const = 0;
    virtual DataList<Review> findByTarget(int targetUserID)     const = 0;
    virtual DataList<Review> findByOrder(int orderID)           const = 0;
    virtual DataList<Review> findAll()                          const = 0;
    virtual bool             deleteReview(int reviewID) = 0;
    virtual double           averageRating(int targetUserID)    const = 0;
    virtual bool             existsForOrder(int orderID)        const = 0;
};

#endif#pragma once
