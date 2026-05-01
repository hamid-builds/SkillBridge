#ifndef SKILLBRIDGE_ADMIN_H
#define SKILLBRIDGE_ADMIN_H

#include "User.h"

class Admin : public User 
{
public:
    Admin(int userID, const std::string& name, const std::string& email, const std::string& passwordHash, double balance = 0.0, const std::string& createdAt = "");

    UserRole getRole() const override;
};

#endif