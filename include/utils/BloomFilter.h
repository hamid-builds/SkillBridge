#ifndef BLOOM_FILTER_H
#define BLOOM_FILTER_H

#include <string>
#include <cstdint>
#include <cstddef>

class BloomFilter 
{
public:
    explicit BloomFilter(size_t numBits = 10000, int numHashes = 4);

    ~BloomFilter();
    BloomFilter(const BloomFilter& other);
    BloomFilter& operator=(const BloomFilter& other);

    void add(const std::string& item);

    bool mightContain(const std::string& item) const;

    void clear();

    size_t getNumBits() const 
    {
        return numBits_; 
    }
    int getNumHashes() const 
    {
        return numHashes_; 
    }

private:
    uint8_t* bits_;
    size_t numBits_;
    size_t numBytes_;
    int numHashes_;

    static uint32_t hashDJB2(const std::string& s);
    static uint32_t hashFNV1a(const std::string& s);

    void setBit(size_t pos);

    bool getBit(size_t pos) const;
};

#endif