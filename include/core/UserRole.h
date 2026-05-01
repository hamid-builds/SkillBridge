#ifndef SKILLBRIDGE_USERROLE_H
#define SKILLBRIDGE_USERROLE_H

#include <string>
#include "Exceptions.h"

enum class UserRole 
{
    CLIENT,
    FREELANCER,
    ADMIN
};

inline std::string roleToString(UserRole role) 
{
    switch (role) 
    {
    case UserRole::CLIENT:     
        return "CLIENT";
    case UserRole::FREELANCER: 
        return "FREELANCER";
    case UserRole::ADMIN:      
        return "ADMIN";
    }
    return "UNKNOWN";
}

inline UserRole stringToRole(const std::string& s) 
{
    if (s == "CLIENT")     
        return UserRole::CLIENT;
    if (s == "FREELANCER") 
        return UserRole::FREELANCER;
    if (s == "ADMIN")      
        return UserRole::ADMIN;
    throw ValidationException("Unknown role string: " + s);
}

#endif