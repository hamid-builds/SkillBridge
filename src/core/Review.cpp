#include "../../include/core/Review.h"
#include "../../include/core/Exceptions.h"
#include <ostream>
#include <cctype>

static bool isBlank(const std::string& s) {
    for (char c : s)
        if (!std::isspace(static_cast<unsigned char>(c))) return false;
    return true;
}

Review::Review(int orderID, int reviewerID, int targetUserID,
    int rating, const std::string& comment)
    : orderID_(orderID),
    reviewerID_(reviewerID),
    targetUserID_(targetUserID),
    rating_(rating),
    comment_(comment)
{
    if (orderID <= 0)
        throw ValidationException("orderID must be positive");
    if (reviewerID <= 0)
        throw ValidationException("reviewerID must be positive");
    if (targetUserID <= 0)
        throw ValidationException("targetUserID must be positive");
    if (reviewerID == targetUserID)
        throw ValidationException("a user cannot review themselves");
    if (rating < 1 || rating > 5)
        throw ValidationException("rating must be between 1 and 5");
    if (isBlank(comment))
        throw ValidationException("comment must not be empty");
}

std::ostream& operator<<(std::ostream& os, const Review& r) {
    os << "[Review #" << r.reviewID_
        << " | Order #" << r.orderID_
        << " | Rating: " << r.rating_ << "/5"
        << " | Sentiment: " << r.sentimentScore_
        << " | " << r.createdAt_ << "]\n"
        << "  " << r.comment_;
    return os;
}