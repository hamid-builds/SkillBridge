#include "managers/SQLiteUserRepository.h"
#include "managers/DatabaseManager.h"
#include "core/User.h"
#include "core/Client.h"
#include "core/Freelancer.h"
#include "core/Admin.h"
#include "core/Exceptions.h"
#include "sqlite3.h"
#include <ctime>
#include <cstdio>
using namespace std;

static std::string nowTimestamp() {
    time_t raw = time(nullptr);
    struct tm tm_buf;
#if defined(_WIN32)
    localtime_s(&tm_buf, &raw);
#else
    localtime_r(&raw, &tm_buf);
#endif
    char buf[64];
    snprintf(buf, sizeof(buf),
        "%04d-%02d-%02d %02d:%02d:%02d",
        tm_buf.tm_year + 1900,
        tm_buf.tm_mon + 1,
        tm_buf.tm_mday,
        tm_buf.tm_hour,
        tm_buf.tm_min,
        tm_buf.tm_sec);
    return std::string(buf);
}

void SQLiteUserRepository::throwPrepareError(const string& sql) 
{
    sqlite3* db = DatabaseManager::getInstance().getConnection();
    string msg = "Failed to prepare SQL: ";
    msg += sqlite3_errmsg(db);
    msg += " | Query: " + sql;
    throw DatabaseException(msg);
}

User* SQLiteUserRepository::buildUserFromRow(sqlite3_stmt* stmt) 
{
    int userID = sqlite3_column_int(stmt, 0);
    const char* nameC = (const char*)sqlite3_column_text(stmt, 1);
    const char* emailC = (const char*)sqlite3_column_text(stmt, 2);
    const char* pwdHashC = (const char*)sqlite3_column_text(stmt, 3);
    const char* roleC = (const char*)sqlite3_column_text(stmt, 4);
    double balance = sqlite3_column_double(stmt, 5);
    const char* createdAtC = (const char*)sqlite3_column_text(stmt, 6);
    const char* portfolioC = (const char*)sqlite3_column_text(stmt, 7);
    const char* skillsC = (const char*)sqlite3_column_text(stmt, 8);
    double avgRating = sqlite3_column_double(stmt, 9);

    string name = nameC ? nameC : "";
    string email = emailC ? emailC : "";
    string passwordHash = pwdHashC ? pwdHashC : "";
    string roleStr = roleC ? roleC : "";
    string createdAt = createdAtC ? createdAtC : "";
    string portfolio = portfolioC ? portfolioC : "";
    string skills = skillsC ? skillsC : "";

    UserRole role = stringToRole(roleStr);

    switch (role) 
    {
    case UserRole::CLIENT:
        return new Client(userID, name, email, passwordHash, balance, createdAt);

    case UserRole::FREELANCER:
        return new Freelancer(userID, name, email, passwordHash, portfolio, skills, avgRating, balance, createdAt);

    case UserRole::ADMIN:
        return new Admin(userID, name, email, passwordHash, balance, createdAt);
    }

    throw DatabaseException("Unhandled UserRole in buildUserFromRow");
}

bool SQLiteUserRepository::saveUser(User* user) 
{
    if (!user) 
    {
        throw ValidationException("Cannot save null User");
    }

    sqlite3* db = DatabaseManager::getInstance().getConnection();

    const string sql =
        "INSERT INTO users "
        "(name, email, passwordHash, role, balance, createdAt, "
        " portfolio, skills, avgRating) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) 
    {
        throwPrepareError(sql);
    }

    string portfolio = "";
    string skills = "";
    double avgRating = 0.0;
    Freelancer* asFreelancer = dynamic_cast<Freelancer*>(user);
    if (asFreelancer) 
    {
        portfolio = asFreelancer->getPortfolio();
        skills = asFreelancer->getSkills();
        avgRating = asFreelancer->getAvgRating();
    }

    string roleStr = roleToString(user->getRole());
    string createdAt = nowTimestamp();


    sqlite3_bind_text(stmt, 1, user->getName().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, user->getEmail().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, user->getPasswordHash().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, roleStr.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 5, user->getBalance());
    sqlite3_bind_text(stmt, 6, createdAt.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 7, portfolio.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 8, skills.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 9, avgRating);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc == SQLITE_CONSTRAINT) 
    {
        return false;
    }
    if (rc != SQLITE_DONE) 
    {
        throw DatabaseException(string("Insert failed: ") + sqlite3_errmsg(db));
    }

    int newID = static_cast<int>(sqlite3_last_insert_rowid(db));
    user->setUserID(newID);
    user->setCreatedAt(createdAt);
    return true;
}

bool SQLiteUserRepository::updateUser(User* user) 
{
    if (!user) 
    {
        throw ValidationException("Cannot update null User");
    }
    if (user->getUserID() <= 0) 
    {
        throw ValidationException("Cannot update user with invalid ID");
    }

    sqlite3* db = DatabaseManager::getInstance().getConnection();

    const string sql =
        "UPDATE users SET "
        "  name = ?, passwordHash = ?, balance = ?, "
        "  portfolio = ?, skills = ?, avgRating = ? "
        "WHERE userID = ?;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) 
    {
        throwPrepareError(sql);
    }

    string portfolio = "";
    string skills = "";
    double avgRating = 0.0;
    Freelancer* asFreelancer = dynamic_cast<Freelancer*>(user);
    if (asFreelancer) 
    {
        portfolio = asFreelancer->getPortfolio();
        skills = asFreelancer->getSkills();
        avgRating = asFreelancer->getAvgRating();
    }

    sqlite3_bind_text(stmt, 1, user->getName().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, user->getPasswordHash().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 3, user->getBalance());
    sqlite3_bind_text(stmt, 4, portfolio.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, skills.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 6, avgRating);
    sqlite3_bind_int(stmt, 7, user->getUserID());

    int rc = sqlite3_step(stmt);
    int changes = sqlite3_changes(db);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) 
    {
        throw DatabaseException(string("Update failed: ") + sqlite3_errmsg(db));
    }
    return changes > 0;
}

User* SQLiteUserRepository::findUserByEmail(const string& email) 
{
    sqlite3* db = DatabaseManager::getInstance().getConnection();

    const string sql =
        "SELECT userID, name, email, passwordHash, role, balance, "
        "       createdAt, portfolio, skills, avgRating "
        "FROM users WHERE email = ?;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) 
    {
        throwPrepareError(sql);
    }

    sqlite3_bind_text(stmt, 1, email.c_str(), -1, SQLITE_TRANSIENT);

    User* result = nullptr;
    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) 
    {
        result = buildUserFromRow(stmt);
    }
    
    sqlite3_finalize(stmt);
    return result;
}

User* SQLiteUserRepository::findUserByID(int userID) 
{
    sqlite3* db = DatabaseManager::getInstance().getConnection();

    const string sql =
        "SELECT userID, name, email, passwordHash, role, balance, "
        "       createdAt, portfolio, skills, avgRating "
        "FROM users WHERE userID = ?;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) 
    {
        throwPrepareError(sql);
    }

    sqlite3_bind_int(stmt, 1, userID);

    User* result = nullptr;
    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) 
    {
        result = buildUserFromRow(stmt);
    }

    sqlite3_finalize(stmt);
    return result;
}

DataList<User*> SQLiteUserRepository::findAllUsers() 
{
    sqlite3* db = DatabaseManager::getInstance().getConnection();

    const string sql =
        "SELECT userID, name, email, passwordHash, role, balance, "
        "       createdAt, portfolio, skills, avgRating "
        "FROM users ORDER BY userID;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) 
    {
        throwPrepareError(sql);
    }

    DataList<User*> users;
    while (sqlite3_step(stmt) == SQLITE_ROW) 
    {
        users.add(buildUserFromRow(stmt));
    }
    sqlite3_finalize(stmt);
    return users;
}

bool SQLiteUserRepository::emailExists(const string& email) 
{
    sqlite3* db = DatabaseManager::getInstance().getConnection();
    const string sql = "SELECT COUNT(*) FROM users WHERE email = ?;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) 
    {
        throwPrepareError(sql);
    }

    sqlite3_bind_text(stmt, 1, email.c_str(), -1, SQLITE_TRANSIENT);

    bool exists = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) 
    {
        exists = (sqlite3_column_int(stmt, 0) > 0);
    }
    sqlite3_finalize(stmt);
    return exists;
}

bool SQLiteUserRepository::deleteUser(int userID) 
{
    sqlite3* db = DatabaseManager::getInstance().getConnection();

    const string sql = "DELETE FROM users WHERE userID = ?;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) 
    {
        throwPrepareError(sql);
    }

    sqlite3_bind_int(stmt, 1, userID);

    int rc = sqlite3_step(stmt);
    int changes = sqlite3_changes(db);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) 
    {
        throw DatabaseException(string("Delete failed: ") + sqlite3_errmsg(db));
    }
    return changes > 0;
}