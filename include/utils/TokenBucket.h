#ifndef TOKEN_BUCKET_H
#define TOKEN_BUCKET_H

#include <chrono>

class TokenBucket 
{
public:
    TokenBucket(double capacity, double refillRatePerSec);

    bool tryConsume();

    double getCurrentTokens();

    double getCapacity() const 
    {
        return capacity_; 
    }
    double getRefillRate() const 
    {
        return refillRate_; 
    }

private:
    double capacity_;
    double refillRate_;
    double tokens_;
    std::chrono::steady_clock::time_point lastRefill_;

    void refill();
};

#endif