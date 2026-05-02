#include "core/User.h"
#include "core/Exceptions.h"

using namespace std;

User::User(int userID, const string& name, const string& email, const string& passwordHash, UserRole role, double balance, const string& createdAt) : userID(userID), name(name), email(email), passwordHash(passwordHash), role(role), balance(balance), createdAt(createdAt)
{
    if (name.empty()) 
    {
        throw ValidationException("User name cannot be empty");
    }
    if (email.empty()) 
    {
        throw ValidationException("User email cannot be empty");
    }
    if (passwordHash.empty()) 
    {
        throw ValidationException("Password hash cannot be empty");
    }
    if (balance < 0.0) 
    {
        throw ValidationException("Initial balance cannot be negative");
    }
}

void User::setName(const string& newName) 
{
    if (newName.empty()) 
    {
        throw ValidationException("User name cannot be empty");
    }
    name = newName;
}

void User::setPasswordHash(const string& newHash) 
{
    if (newHash.empty()) 
    {
        throw ValidationException("Password hash cannot be empty");
    }
    passwordHash = newHash;
}

void User::deposit(double amount) 
{
    if (amount <= 0.0) 
    {
        throw ValidationException("Deposit amount must be positive");
    }
    balance += amount;
}



void User::withdraw(double amount) 
{
    if (amount <= 0.0) 
    {
        throw ValidationException("Withdrawal amount must be positive");
    }
    if (amount > balance) 
    {
      
        throw InsufficientFundsException(amount, balance);
    }
    balance -= amount;
}