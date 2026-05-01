#ifndef SKILLBRIDGE_USERMANAGER_H
#define SKILLBRIDGE_USERMANAGER_H

#include <string>
#include "IUserRepository.h"
#include "utils/IPasswordHasher.h"
#include "utils/BloomFilter.h"
#include "utils/HashMap.h"
#include "utils/TokenBucket.h"
#include "core/UserRole.h"

class User;
class UserManager 
{
private:
    IUserRepository* repo_;
    IPasswordHasher* hasher_;

    BloomFilter bloomFilter_;
    HashMap<std::string, TokenBucket> loginBuckets_;
    HashMap<std::string, TokenBucket> registerBuckets_;

    User* currentUser_;

    void validateName(const std::string& name);
    void validateEmail(const std::string& email);
    void validatePassword(const std::string& plaintext);
    void enforceRateLimit(const std::string& email, HashMap<std::string, TokenBucket>& buckets, double capacity, double refillPerSec, const std::string& operation);

    void setCurrentUser(User* newUser);

public:
    UserManager(IUserRepository* repo, IPasswordHasher* hasher);
    ~UserManager();
    UserManager(const UserManager&) = delete;
    UserManager& operator=(const UserManager&) = delete;
    User* registerUser(const std::string& name, const std::string& email, const std::string& plaintextPassword, UserRole role);
    User* login(const std::string& email, const std::string& plaintextPassword);

    void logout();

    bool updateProfile(const std::string& newName);

    bool changePassword(const std::string& oldPassword, const std::string& newPassword);

    bool deleteAccount(const std::string& plaintextPassword);

    User* getCurrentUser() const 
    {
        return currentUser_; 
    }
    bool isLoggedIn() const 
    {
        return currentUser_ != nullptr;
    }
};

#endif