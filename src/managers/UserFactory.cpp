#include "managers/UserFactory.h"
#include "core/Client.h"
#include "core/Freelancer.h"
#include "core/Admin.h"
#include "core/Exceptions.h"

using namespace std;

User* UserFactory::create(UserRole role, const string& name, const string& email, const string& passwordHash) 
{
    switch (role) 
    {
    case UserRole::CLIENT:
        return new Client(0, name, email, passwordHash);
    case UserRole::FREELANCER:
        return new Freelancer(0, name, email, passwordHash);
    case UserRole::ADMIN:
        return new Admin(0, name, email, passwordHash);
    default:
        throw ValidationException("UserFactory: unknown UserRole value");
    }
}