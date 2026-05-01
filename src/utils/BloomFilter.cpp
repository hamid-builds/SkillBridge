#include "utils/BloomFilter.h"
#include <stdexcept>
#include <cstring>

using namespace std;

BloomFilter::BloomFilter(size_t numBits, int numHashes) : bits_(nullptr), numBits_(numBits), numBytes_(0), numHashes_(numHashes) 
{
    if (numBits == 0 || numHashes <= 0) 
    {
        throw invalid_argument("BloomFilter: numBits and numHashes must be positive");
    }
    numBytes_ = (numBits + 7) / 8;
    bits_ = new uint8_t[numBytes_];
    memset(bits_, 0, numBytes_);
}

BloomFilter::~BloomFilter() 
{
    delete[] bits_;
}

BloomFilter::BloomFilter(const BloomFilter& other) : bits_(nullptr), numBits_(other.numBits_), numBytes_(other.numBytes_), numHashes_(other.numHashes_) 
{
    bits_ = new uint8_t[numBytes_];
    memcpy(bits_, other.bits_, numBytes_);
}

BloomFilter& BloomFilter::operator=(const BloomFilter& other) 
{
    if (this == &other) 
        return *this;
    delete[] bits_;
    numBits_ = other.numBits_;
    numBytes_ = other.numBytes_;
    numHashes_ = other.numHashes_;
    bits_ = new uint8_t[numBytes_];
    memcpy(bits_, other.bits_, numBytes_);
    return *this;
}

uint32_t BloomFilter::hashDJB2(const string& s) 
{
    uint32_t h = 5381;
    for (char c : s) 
    {
        h = ((h << 5) + h) + static_cast<uint8_t>(c);
    }
    return h;
}

uint32_t BloomFilter::hashFNV1a(const string& s) 
{
    uint32_t h = 2166136261u;
    for (char c : s) 
    {
        h ^= static_cast<uint8_t>(c);
        h *= 16777619u;
    }
    return h;
}

void BloomFilter::setBit(size_t pos) 
{
    bits_[pos / 8] |= (uint8_t(1) << (pos % 8));
}

bool BloomFilter::getBit(size_t pos) const 
{
    return (bits_[pos / 8] & (uint8_t(1) << (pos % 8))) != 0;
}

void BloomFilter::add(const string& item) 
{
    uint32_t h1 = hashDJB2(item);
    uint32_t h2 = hashFNV1a(item);
    for (int i = 0; i < numHashes_; i++) 
    {
        size_t pos = (h1 + static_cast<uint32_t>(i) * h2) % numBits_;
        setBit(pos);
    }
}

bool BloomFilter::mightContain(const string& item) const 
{
    uint32_t h1 = hashDJB2(item);
    uint32_t h2 = hashFNV1a(item);
    for (int i = 0; i < numHashes_; i++) 
    {
        size_t pos = (h1 + static_cast<uint32_t>(i) * h2) % numBits_;
        if (!getBit(pos)) 
        {
            return false;
        }
    }
    return true;
}

void BloomFilter::clear() 
{
    memset(bits_, 0, numBytes_);
}