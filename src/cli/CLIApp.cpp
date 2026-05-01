#include "cli/CLIApp.h"
#include "managers/UserManager.h"
#include "core/User.h"
#include "core/UserRole.h"
#include "core/Exceptions.h"

#include <iostream>
#include <limits>
#include <string>

using namespace std;

CLIApp::CLIApp(UserManager& userManager) : userManager_(userManager), running_(true) 
{

}

void CLIApp::run() 
{
    cout << "\n";
    cout << "========================================\n";
    cout << "    Welcome to SkillBridge (CLI)\n";
    cout << "========================================\n";

    while (running_) 
    {
        if (userManager_.isLoggedIn()) 
        {
            showLoggedInMenu();
        }
        else 
        {
            showLoggedOutMenu();
        }
    }

    cout << "\nGoodbye.\n";
}

int CLIApp::readMenuChoice() 
{
    int choice = 0;
    cout << "> ";
    if (!(cin >> choice)) 
    {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return -1;
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    return choice;
}

string CLIApp::readLine(const string& prompt) 
{
    cout << prompt;
    string line;
    getline(cin, line);
    return line;
}

void CLIApp::printHeader(const string& title) 
{
    cout << "\n-- " << title << " --\n";
}

void CLIApp::showLoggedOutMenu() 
{
    cout << "\n";
    cout << "1. Register\n";
    cout << "2. Login\n";
    cout << "3. Exit\n";

    int choice = readMenuChoice();
    switch (choice) 
    {
    case 1: 
        doRegister(); 
        break;
    case 2: 
        doLogin(); 
        break;
    case 3: 
        running_ = false; 
        break;
    default: 
        cout << "Invalid choice. Try again.\n"; 
        break;
    }
}

void CLIApp::doRegister() 
{
    printHeader("Register");

    try 
    {
        string name = readLine("Name:     ");
        string email = readLine("Email:    ");
        string pwd = readLine("Password: ");

        cout << "Role:\n";
        cout << "  1. Client\n";
        cout << "  2. Freelancer\n";
        cout << "  3. Admin\n";
        int rc = readMenuChoice();

        UserRole role;
        switch (rc) 
        {
        case 1: 
            role = UserRole::CLIENT; 
            break;
        case 2: 
            role = UserRole::FREELANCER; 
            break;
        case 3: 
            role = UserRole::ADMIN; 
            break;
        default:
            cout << "Invalid role.\n";
            return;
        }

        User* u = userManager_.registerUser(name, email, pwd, role);
        cout << "\nRegistered successfully. Welcome, " << u->getName() << ".\n";
    }
    catch (const SkillBridgeException& e) 
    {
        cout << "\nError: " << e.what() << "\n";
    }
}

void CLIApp::doLogin() 
{
    printHeader("Login");

    try 
    {
        string email = readLine("Email:    ");
        string pwd = readLine("Password: ");

        User* u = userManager_.login(email, pwd);
        cout << "\nLogged in as " << u->getName() << ".\n";
    }
    catch (const SkillBridgeException& e) 
    {
        cout << "\nError: " << e.what() << "\n";
    }
}

void CLIApp::showLoggedInMenu() 
{
    User* u = userManager_.getCurrentUser();
    cout << "\n=== Welcome, " << u->getName() << " ===\n";
    cout << "1. View my profile\n";
    cout << "2. Update name\n";
    cout << "3. Change password\n";
    cout << "4. Delete account\n";
    cout << "5. Logout\n";
    cout << "6. Exit\n";

    int choice = readMenuChoice();
    switch (choice) 
    {
    case 1: 
        doViewProfile(); 
        break;
    case 2: 
        doUpdateName(); 
        break;
    case 3: 
        doChangePassword(); 
        break;
    case 4: 
        doDeleteAccount(); 
        break;
    case 5: 
        doLogout(); 
        break;
    case 6: 
        running_ = false; 
        break;
    default: 
        cout << "Invalid choice. Try again.\n"; 
        break;
    }
}

void CLIApp::doViewProfile() 
{
    printHeader("Profile");

    User* u = userManager_.getCurrentUser();
    if (!u) 
    {
        cout << "Not logged in.\n";
        return;
    }

    string roleStr;
    switch (u->getRole()) 
    {
    case UserRole::CLIENT:     
        roleStr = "Client"; 
        break;
    case UserRole::FREELANCER: 
        roleStr = "Freelancer"; 
        break;
    case UserRole::ADMIN: 
        roleStr = "Admin";
        break;
    default:
        roleStr = "Unknown"; 
        break;
    }

    cout << "User ID:   " << u->getUserID() << "\n";
    cout << "Name:      " << u->getName() << "\n";
    cout << "Email:     " << u->getEmail() << "\n";
    cout << "Role:      " << roleStr << "\n";
    cout << "Balance:   " << u->getBalance() << "\n";
    cout << "Joined:    " << u->getCreatedAt() << "\n";
}

void CLIApp::doUpdateName()
{
    printHeader("Update Name");

    try 
    {
        string newName = readLine("New name: ");
        if (userManager_.updateProfile(newName)) 
        {
            cout << "\nName updated successfully.\n";
        }
        else 
        {
            cout << "\nUpdate failed.\n";
        }
    }
    catch (const SkillBridgeException& e) 
    {
        cout << "\nError: " << e.what() << "\n";
    }
}

void CLIApp::doChangePassword() 
{
    printHeader("Change Password");

    try 
    {
        string oldPwd = readLine("Current password: ");
        string newPwd = readLine("New password:     ");

        if (userManager_.changePassword(oldPwd, newPwd)) 
        {
            cout << "\nPassword changed successfully.\n";
        }
        else 
        {
            cout << "\nPassword change failed.\n";
        }
    }
    catch (const SkillBridgeException& e) 
    {
        cout << "\nError: " << e.what() << "\n";
    }
}

void CLIApp::doDeleteAccount() 
{
    printHeader("Delete Account");

    try 
    {
        string confirm = readLine("Type your password to confirm deletion: ");

        if (userManager_.deleteAccount(confirm)) 
        {
            cout << "\nAccount deleted. Goodbye.\n";
        }
        else 
        {
            cout << "\nDelete failed.\n";
        }
    }
    catch (const SkillBridgeException& e) 
    {
        cout << "\nError: " << e.what() << "\n";
    }
}

void CLIApp::doLogout() 
{
    userManager_.logout();
    cout << "\nLogged out.\n";
}