#ifndef USER_FACTORY_H
#define USER_FACTORY_H

#include <string>
#include "core/User.h"
#include "core/UserRole.h"

class UserFactory 
{
public:
    static User* create(UserRole role, const std::string& name, const std::string& email, const std::string& passwordHash);

    UserFactory() = delete;
};

#endif