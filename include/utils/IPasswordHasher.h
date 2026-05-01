#ifndef IPASSWORD_HASHER_H
#define IPASSWORD_HASHER_H

#include <string>

class IPasswordHasher 
{
public:
    virtual ~IPasswordHasher() = default;

    virtual std::string hash(const std::string& plaintext) const = 0;

    virtual bool verify(const std::string& plaintext, const std::string& storedHash) const = 0;
};

#endif