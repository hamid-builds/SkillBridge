#include "core/Freelancer.h"
#include "core/Exceptions.h"

using namespace std;

Freelancer::Freelancer(int userID, const string& name, const string& email, const string& passwordHash, const string& portfolio, const string& skills, double avgRating, double balance, const string& createdAt) : User(userID, name, email, passwordHash, UserRole::FREELANCER, balance, createdAt), portfolio(portfolio), skills(skills), avgRating(avgRating)
{
    if (avgRating < 0.0 || avgRating > 5.0) 
    {
        throw ValidationException("Average rating must be between 0 and 5");
    }
}

UserRole Freelancer::getRole() const 
{
    return UserRole::FREELANCER;
}

void Freelancer::setPortfolio(const string& newPortfolio) 
{
    if (newPortfolio.length() > 2000) 
    {
        throw ValidationException("Portfolio cannot exceed 2000 characters");
    }
    portfolio = newPortfolio;
}

void Freelancer::setSkills(const string& newSkills) 
{
    if (newSkills.length() > 500) 
    {
        throw ValidationException("Skills list cannot exceed 500 characters");
    }
    skills = newSkills;
}

void Freelancer::updateAvgRating(double newAvg) 
{
    if (newAvg < 0.0 || newAvg > 5.0) 
    {
        throw ValidationException("Average rating must be between 0 and 5");
    }
    avgRating = newAvg;
}