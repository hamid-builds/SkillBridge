#ifndef SKILLBRIDGE_FREELANCER_H
#define SKILLBRIDGE_FREELANCER_H

#include "User.h"

class Freelancer : public User 
{
private:
    std::string portfolio;
    std::string skills;
    double avgRating;

public:
    Freelancer(int userID, const std::string& name, const std::string& email, const std::string& passwordHash, const std::string& portfolio = "", const std::string& skills = "", double avgRating = 0.0, double balance = 0.0, const std::string& createdAt = "");

    UserRole getRole() const override;

    std::string getPortfolio() const 
    {
        return portfolio; 
    }
    std::string getSkills() const 
    { 
        return skills; 
    }
    double getAvgRating() const 
    {
        return avgRating; 
    }

    void setPortfolio(const std::string& newPortfolio);
    void setSkills(const std::string& newSkills);

    void updateAvgRating(double newAvg);
};

#endif