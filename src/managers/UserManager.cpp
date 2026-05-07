#include "managers/UserManager.h"
#include "managers/UserFactory.h"
#include "core/User.h"
#include "core/Freelancer.h"
#include "core/Exceptions.h"

#include <stdexcept>
#include <cctype>

using namespace std;

static constexpr double LOGIN_CAPACITY = 5.0;
static constexpr double LOGIN_REFILL = 5.0 / 60.0;
static constexpr double REGISTER_CAPACITY = 3.0;
static constexpr double REGISTER_REFILL = 3.0 / 60.0;

UserManager::UserManager(IUserRepository* repo, IPasswordHasher* hasher) : repo_(repo), hasher_(hasher), bloomFilter_(10000, 4), loginBuckets_(64), registerBuckets_(64), currentUser_(nullptr) 
{
    if (!repo_ || !hasher_) 
    {
        throw invalid_argument("UserManager: repo and hasher must not be null");
    }
}

UserManager::~UserManager() 
{
    delete currentUser_;
    currentUser_ = nullptr;
}

void UserManager::validateName(const string& name) 
{
    if (name.empty()) 
    {
        throw ValidationException("name cannot be empty");
    }
    if (name.size() > 100) 
    {
        throw ValidationException("name too long (max 100 chars)");
    }
}

void UserManager::validateEmail(const string& email) 
{
    if (email.empty()) 
    {
        throw ValidationException("email cannot be empty");
    }
    size_t atPos = email.find('@');
    if (atPos == string::npos) 
    {
        throw ValidationException("email must contain '@'");
    }
    size_t dotPos = email.find('.', atPos);
    if (dotPos == string::npos) 
    {
        throw ValidationException("email must contain '.' after '@'");
    }
    if (atPos == 0 || dotPos == email.size() - 1) 
    {
        throw ValidationException("email format invalid");
    }
}

void UserManager::validatePassword(const string& plaintext) 
{
    if (plaintext.size() < 8) 
    {
        throw ValidationException("password must be at least 8 characters");
    }
    bool hasLetter = false;
    bool hasDigit = false;
    for (char c : plaintext) 
    {
        if (isalpha(static_cast<unsigned char>(c))) 
            hasLetter = true;
        if (isdigit(static_cast<unsigned char>(c))) 
            hasDigit = true;
    }
    if (!hasLetter) 
    {
        throw ValidationException("password must contain at least one letter");
    }
    if (!hasDigit) 
    {
        throw ValidationException("password must contain at least one digit");
    }
}

void UserManager::enforceRateLimit(const string& email, HashMap<string, TokenBucket>& buckets, double capacity, double refillPerSec, const string& operation) 
{
    if (!buckets.contains(email)) 
    {
        buckets.put(email, TokenBucket(capacity, refillPerSec));
    }
    TokenBucket* bucket = buckets.get(email);
    if (!bucket->tryConsume()) 
    {
        throw RateLimitException("too many " + operation + " attempts for " + email);
    }
}

void UserManager::setCurrentUser(User* newUser) 
{
    if (currentUser_ == newUser) 
        return;
    delete currentUser_;
    currentUser_ = newUser;
}

User* UserManager::registerUser(const string& name, const string& email, const string& plaintextPassword, UserRole role) 
{
    validateName(name);
    validateEmail(email);
    validatePassword(plaintextPassword);
    
    if (role == UserRole::ADMIN) 
    {
        throw ValidationException("invalid role for registration");
    }

    enforceRateLimit(email, registerBuckets_, REGISTER_CAPACITY, REGISTER_REFILL, "registration");

    if (bloomFilter_.mightContain(email)) 
    {
        if (repo_->emailExists(email)) 
        {
            throw DuplicateEntryException("email already registered: " + email);
        }
    }

    string hash = hasher_->hash(plaintextPassword);

    User* newUser = UserFactory::create(role, name, email, hash);

    if (role == UserRole::CLIENT) 
    {
        newUser->deposit(1000.0);
    }

    if (!repo_->saveUser(newUser)) 
    {
        delete newUser;
        throw DatabaseException("failed to save new user");
    }

    bloomFilter_.add(email);

    setCurrentUser(newUser);
    return currentUser_;
}

User* UserManager::login(const string& email, const string& plaintextPassword) 
{
    if (email.empty()) 
    {
        throw ValidationException("email cannot be empty");
    }
    if (plaintextPassword.empty()) 
    {
        throw ValidationException("password cannot be empty");
    }

    enforceRateLimit(email, loginBuckets_, LOGIN_CAPACITY, LOGIN_REFILL, "login");

    User* found = repo_->findUserByEmail(email);
    if (!found) 
    {
        throw AuthenticationException("unknown email: " + email);
    }

    if (!hasher_->verify(plaintextPassword, found->getPasswordHash())) 
    {
        delete found;
        throw AuthenticationException("incorrect password");
    }

    setCurrentUser(found);
    return currentUser_;
}

void UserManager::logout() 
{
    setCurrentUser(nullptr);
}

bool UserManager::updateProfile(const string& newName) 
{
    if (!currentUser_) 
    {
        throw AuthenticationException("must be logged in to update profile");
    }
    validateName(newName);

    currentUser_->setName(newName);
    return repo_->updateUser(currentUser_);
}

bool UserManager::changePassword(const string& oldPassword, const string& newPassword) 
{
    if (!currentUser_) 
    {
        throw AuthenticationException("must be logged in to change password");
    }
    if (!hasher_->verify(oldPassword, currentUser_->getPasswordHash())) 
    {
        throw AuthenticationException("old password is incorrect");
    }
    validatePassword(newPassword);

    currentUser_->setPasswordHash(hasher_->hash(newPassword));
    return repo_->updateUser(currentUser_);
}

bool UserManager::deleteAccount(const string& plaintextPassword) 
{
    if (!currentUser_) 
    {
        throw AuthenticationException("must be logged in to delete account");
    }
    if (!hasher_->verify(plaintextPassword, currentUser_->getPasswordHash())) 
    {
        throw AuthenticationException("password verification failed");
    }

    int id = currentUser_->getUserID();
    bool ok = repo_->deleteUser(id);
    if (ok) 
    {
        logout();
    }
    return ok;
}

bool UserManager::depositToBalance(double amount) 
{
    if (!currentUser_) 
    {
        throw AuthenticationException("must be logged in to deposit funds");
    }
    if (amount <= 0.0) 
    {
        throw ValidationException("deposit amount must be positive");
    }
    if (amount > 1000000.0) 
    {
        throw ValidationException("deposit amount exceeds maximum (Rs. 1,000,000)");
    }

    currentUser_->deposit(amount);

    if (!repo_->updateUser(currentUser_)) 
    {
        currentUser_->withdraw(amount);
        throw DatabaseException("failed to persist deposit for userID=" + std::to_string(currentUser_->getUserID()));
    }

    return true;
}

void UserManager::requireAdmin(int adminID) const 
{
    User* u = repo_->findUserByID(adminID);
    if (!u) 
    {
        throw UnauthorizedException("admin access required: user not found");
    }
    UserRole role = u->getRole();
    delete u;
    if (role != UserRole::ADMIN) 
    {
        throw UnauthorizedException("admin access required");
    }
}

DataList<User*> UserManager::adminListAllUsers(int currentUserID) 
{
    requireAdmin(currentUserID);
    return repo_->findAllUsers();
}

bool UserManager::adminDeleteUser(int currentUserID, int targetUserID) 
{
    requireAdmin(currentUserID);

    if (targetUserID == currentUserID) 
    {
        throw ValidationException("cannot delete your own admin account through admin panel; ""use the regular delete account flow with password");
    }

    return repo_->deleteUser(targetUserID);
}

bool UserManager::seedAdmin(const string& name, const string& email, const string& plaintextPassword) 
{
    validateName(name);
    validateEmail(email);
    validatePassword(plaintextPassword);

    if (repo_->emailExists(email)) 
    {
        return false;
    }

    string hash = hasher_->hash(plaintextPassword);

    User* admin = UserFactory::create(UserRole::ADMIN, name, email, hash);

    if (!repo_->saveUser(admin)) 
    {
        delete admin;
        throw DatabaseException("failed to save seeded admin");
    }

    bloomFilter_.add(email);

    delete admin;
    return true;
}

User* UserManager::authenticate(const string& email, const string& plaintextPassword) 
{
    if (email.empty()) 
    {
        throw ValidationException("email cannot be empty");
    }
    if (plaintextPassword.empty()) 
    {
        throw ValidationException("password cannot be empty");
    }

    enforceRateLimit(email, loginBuckets_, LOGIN_CAPACITY, LOGIN_REFILL, "login");

    User* found = repo_->findUserByEmail(email);
    if (!found) 
    {
        throw AuthenticationException("unknown email: " + email);
    }

    if (!hasher_->verify(plaintextPassword, found->getPasswordHash())) 
    {
        delete found;
        throw AuthenticationException("incorrect password");
    }

    return found;
}

User* UserManager::findUserByID(int userID) const 
{
    return repo_->findUserByID(userID);
}

bool UserManager::updateProfileForUser(int userID, const string& newName) {
    validateName(newName);

    User* u = repo_->findUserByID(userID);
    if (!u) {
        throw AuthenticationException(
            "user not found: id=" + to_string(userID));
    }

    u->setName(newName);
    bool ok = repo_->updateUser(u);
    delete u;
    return ok;
}

bool UserManager::updateFreelancerFields(int userID,
    const string& portfolio,
    const string& skills) {
    User* u = repo_->findUserByID(userID);
    if (!u) {
        throw AuthenticationException(
            "user not found: id=" + to_string(userID));
    }

    Freelancer* f = dynamic_cast<Freelancer*>(u);
    if (!f) {
        delete u;
        throw UnauthorizedException(
            "only freelancers can update portfolio or skills");
    }

    try {
        f->setPortfolio(portfolio);
        f->setSkills(skills);
    }
    catch (...) {
        delete u;
        throw;
    }

    bool ok = repo_->updateUser(u);
    delete u;
    return ok;
}

bool UserManager::changePasswordForUser(int userID,
    const string& oldPassword,
    const string& newPassword) {
    User* u = repo_->findUserByID(userID);
    if (!u) {
        throw AuthenticationException(
            "user not found: id=" + to_string(userID));
    }

    if (!hasher_->verify(oldPassword, u->getPasswordHash())) {
        delete u;
        throw AuthenticationException("old password is incorrect");
    }

    try {
        validatePassword(newPassword);
    }
    catch (...) {
        delete u;
        throw;
    }

    u->setPasswordHash(hasher_->hash(newPassword));
    bool ok = repo_->updateUser(u);
    delete u;
    return ok;
}

bool UserManager::deleteAccountForUser(int userID,
    const string& plaintextPassword) {
    User* u = repo_->findUserByID(userID);
    if (!u) {
        throw AuthenticationException(
            "user not found: id=" + to_string(userID));
    }

    if (!hasher_->verify(plaintextPassword, u->getPasswordHash())) {
        delete u;
        throw AuthenticationException("password verification failed");
    }

    delete u;
    return repo_->deleteUser(userID);
}

bool UserManager::depositForUser(int userID, double amount) {
    if (amount <= 0.0) {
        throw ValidationException("deposit amount must be positive");
    }
    if (amount > 1000000.0) {
        throw ValidationException(
            "deposit amount exceeds maximum (Rs. 1,000,000)");
    }

    User* u = repo_->findUserByID(userID);
    if (!u) {
        throw AuthenticationException(
            "user not found: id=" + to_string(userID));
    }

    if (u->getRole() != UserRole::CLIENT) {
        delete u;
        throw UnauthorizedException("only clients can deposit funds");
    }

    u->deposit(amount);
    if (!repo_->updateUser(u)) {
        u->withdraw(amount);
        delete u;
        throw DatabaseException(
            "failed to persist deposit for userID=" + to_string(userID));
    }

    double newBalance = u->getBalance();
    delete u;
    return true;
}