#ifndef SKILLBRIDGE_CLIAPP_H
#define SKILLBRIDGE_CLIAPP_H

#include <string>

class UserManager;

class CLIApp 
{
private:
    UserManager& userManager_;
    bool running_;

    void showLoggedOutMenu();
    void showLoggedInMenu();

    void doRegister();
    void doLogin();


    void doViewProfile();
    void doUpdateName();
    void doChangePassword();
    void doDeleteAccount();
    void doLogout();

    int readMenuChoice();

    std::string readLine(const std::string& prompt);

    void printHeader(const std::string& title);

public:
    explicit CLIApp(UserManager& userManager);

    void run();
};

#endif