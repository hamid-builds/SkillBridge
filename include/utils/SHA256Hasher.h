#ifndef SHA256_HASHER_H
#define SHA256_HASHER_H

#include "utils/IPasswordHasher.h"
#include <string>
#include <cstdint>

class SHA256Hasher : public IPasswordHasher 
{
public:
    SHA256Hasher() = default;
    ~SHA256Hasher() override = default;

    std::string hash(const std::string& plaintext) const override;
    bool verify(const std::string& plaintext, const std::string& storedHash) const override;

private:
    static uint32_t rotr(uint32_t x, uint32_t n);
    
    static void processBlock(const uint8_t* block, uint32_t state[8]);

    static std::string toHex(const uint8_t* digest, size_t len);
};

#endif