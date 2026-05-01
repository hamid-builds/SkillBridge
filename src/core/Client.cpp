#include "core/Client.h"

using namespace std;

Client::Client(int userID, const string& name, const string& email, const string& passwordHash, double balance, const string& createdAt) : User(userID, name, email, passwordHash, UserRole::CLIENT, balance, createdAt)
{
    
}

UserRole Client::getRole() const 
{
    return UserRole::CLIENT;
}