#ifndef SKILLBRIDGE_SQLITEUSERREPOSITORY_H
#define SKILLBRIDGE_SQLITEUSERREPOSITORY_H

#include "IUserRepository.h"

struct sqlite3_stmt;

class SQLiteUserRepository : public IUserRepository 
{
public:
    SQLiteUserRepository() = default;
    ~SQLiteUserRepository() override = default;

    bool saveUser(User* user) override;
    bool updateUser(User* user) override;
    User* findUserByEmail(const std::string& email) override;
    User* findUserByID(int userID) override;
    DataList<User*> findAllUsers() override;
    bool emailExists(const std::string& email) override;
    bool deleteUser(int userID) override;

private:
    User* buildUserFromRow(sqlite3_stmt* stmt);
    void throwPrepareError(const std::string& sql);
};

#endif