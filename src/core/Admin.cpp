#include "core/Admin.h"

using namespace std;

Admin::Admin(int userID, const string& name, const string& email, const string& passwordHash, double balance, const string& createdAt) : User(userID, name, email, passwordHash, UserRole::ADMIN, balance, createdAt)
{

}

UserRole Admin::getRole() const 
{
    return UserRole::ADMIN;
}