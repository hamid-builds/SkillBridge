#ifndef SKILLBRIDGE_REVIEW_H
#define SKILLBRIDGE_REVIEW_H

#include <string>



class Review {
public:
    Review() = default;

    Review(int orderID, int reviewerID, int targetUserID,
        int rating, const std::string& comment);

    int         getReviewID()       const
    {
        return reviewID_; 
    }
    int         getOrderID()        const 
    {
        return orderID_; 
    }
    int         getReviewerID()     const
    { return reviewerID_; }
    int         getTargetUserID()   const 
    {
        return targetUserID_; 
    }
    int         getRating()         const
    {
        return rating_; 
    }
    double      getSentimentScore() const
    {
        return sentimentScore_; 
    }
    const std::string& getComment()   const
    {
        return comment_; 
    }
    const std::string& getCreatedAt() const 
    { 
        return createdAt_;
    }


    void setReviewID(int id) 
    {
        reviewID_ = id;
    }
    void setOrderID(int id) 
    {
        orderID_ = id;
    }
    void setReviewerID(int id)
    {
        reviewerID_ = id; 
    }
    void setTargetUserID(int id) 
    {
        targetUserID_ = id; 
    }
    void setRating(int r) 
    {
        rating_ = r;
    }
    void setSentimentScore(double s)
    {
        sentimentScore_ = s;
    }
    void setComment(const std::string& c) 
    {
        comment_ = c; 
    }
    void setCreatedAt(const std::string& t) 
    {
        createdAt_ = t; 
    }

    bool operator==(const Review& other) const
    {
        return reviewID_ == other.reviewID_;
    }
    bool operator< (const Review& other) const 
    {
        return createdAt_ < other.createdAt_; 
    }

    friend std::ostream& operator<<(std::ostream& os, const Review& r);

private:
    int         reviewID_ = 0;
    int         orderID_ = 0;
    int         reviewerID_ = 0;
    int         targetUserID_ = 0;
    int         rating_ = 0;
    double      sentimentScore_ = 0.0;
    std::string comment_;
    std::string createdAt_;
};

#endif