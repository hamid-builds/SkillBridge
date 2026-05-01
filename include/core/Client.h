#ifndef SKILLBRIDGE_CLIENT_H
#define SKILLBRIDGE_CLIENT_H

#include "User.h"

class Client : public User 
{
public:
    Client(int userID, const std::string& name, const std::string& email, const std::string& passwordHash, double balance = 0.0, const std::string& createdAt = "");

    UserRole getRole() const override;
};

#endif