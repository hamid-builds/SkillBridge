#include "utils/TokenBucket.h"
#include <stdexcept>

using namespace std;
using namespace std::chrono;

TokenBucket::TokenBucket(double capacity, double refillRatePerSec) : capacity_(capacity), refillRate_(refillRatePerSec), tokens_(capacity), lastRefill_(steady_clock::now()) 
{
    if (capacity <= 0.0 || refillRatePerSec <= 0.0) 
    {
        throw invalid_argument("TokenBucket: capacity and refillRate must be positive");
    }
}

void TokenBucket::refill() 
{
    auto now = steady_clock::now();
    duration<double> elapsed = now - lastRefill_;
    double secondsElapsed = elapsed.count();

    tokens_ += secondsElapsed * refillRate_;

    if (tokens_ > capacity_) 
        tokens_ = capacity_;

    lastRefill_ = now;
}

bool TokenBucket::tryConsume() 
{
    refill();
    if (tokens_ >= 1.0) 
    {
        tokens_ -= 1.0;
        return true;
    }
    return false;
}

double TokenBucket::getCurrentTokens() 
{
    refill();
    return tokens_;
}