#ifndef SKILLBRIDGE_USER_H
#define SKILLBRIDGE_USER_H

#include <string>
#include "UserRole.h"

class User 
{
protected:
    int userID;
    std::string name;
    std::string email;
    std::string passwordHash;
    UserRole role;
    double balance;
    std::string createdAt;
public:
    User(int userID, const std::string& name, const std::string& email, const std::string& passwordHash, UserRole role, double balance = 0.0, const std::string& createdAt = "");

    virtual ~User() = default;

    virtual UserRole getRole() const = 0;

    int getUserID() const 
    {
        return userID; 
    }
    std::string getName() const
    {
        return name; 
    }
    std::string getEmail() const
    {
        return email; 
    }
    std::string getPasswordHash() const 
    { 
        return passwordHash; 
    }
    double getBalance() const 
    {
        return balance; 
    }
    std::string getCreatedAt() const 
    {
        return createdAt; 
    }
    
    void setName(const std::string& newName);
    void setPasswordHash(const std::string& newHash);
    
    void deposit(double amount);
    void withdraw(double amount);

    void setUserID(int id) 
    {
        userID = id; 
    }
};

#endif