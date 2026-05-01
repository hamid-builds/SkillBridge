#ifndef SKILLBRIDGE_IUSERREPOSITORY_H
#define SKILLBRIDGE_IUSERREPOSITORY_H

#include <string>
#include"utils/DataList.h"

class User;

class IUserRepository 
{
public:
    virtual ~IUserRepository() = default;

    virtual bool saveUser(User* user) = 0;

    virtual bool updateUser(User* user) = 0;

    virtual User* findUserByEmail(const std::string& email) = 0;

    virtual User* findUserByID(int userID) = 0;

    virtual DataList<User*> findAllUsers() = 0;

    virtual bool emailExists(const std::string& email) = 0;

    virtual bool deleteUser(int userID) = 0;
};

#endif