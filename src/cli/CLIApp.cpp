#include "cli/CLIApp.h"
#include "managers/UserManager.h"
#include "managers/GigManager.h"
#include "core/User.h"
#include "core/UserRole.h"
#include "core/Gig.h"
#include "core/GigCategory.h"
#include "core/Exceptions.h"
#include "utils/DataList.h"

#include <iostream>
#include <limits>
#include <string>
#include <stdexcept>

using namespace std;

CLIApp::CLIApp(UserManager& userManager, GigManager& gigManager)
    : userManager_(userManager), gigManager_(gigManager), running_(true) {
}



void CLIApp::run() {
    cout << "\n";
    cout << "========================================\n";
    cout << "    Welcome to SkillBridge (CLI)\n";
    cout << "========================================\n";

    while (running_) {
        if (userManager_.isLoggedIn()) {
            showLoggedInMenu();
        }
        else {
            showLoggedOutMenu();
        }
    }

    cout << "\nGoodbye.\n";
}



int CLIApp::readMenuChoice() {
    int choice = 0;
    cout << "> ";
    if (!(cin >> choice)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return -1;
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    return choice;
}

int CLIApp::readInt(const string& prompt) {
    cout << prompt;
    string line;
    getline(cin, line);
    try {
        return stoi(line);
    }
    catch (...) {
        return -1;
    }
}

double CLIApp::readDouble(const string& prompt) {
    cout << prompt;
    string line;
    getline(cin, line);
    try {
        return stod(line);
    }
    catch (...) {
       
        return -1.0;
    }
}

string CLIApp::readLine(const string& prompt) {
    cout << prompt;
    string line;
    getline(cin, line);
    return line;
}

void CLIApp::printHeader(const string& title) {
    cout << "\n-- " << title << " --\n";
}


void CLIApp::printGigSummary(const Gig& gig) {
    string roleStr;
    switch (gig.getCategory()) {
    case GigCategory::CODING:    roleStr = "CODING";    break;
    case GigCategory::DESIGN:    roleStr = "DESIGN";    break;
    case GigCategory::WRITING:   roleStr = "WRITING";   break;
    case GigCategory::MARKETING: roleStr = "MARKETING"; break;
    case GigCategory::TUTORING:  roleStr = "TUTORING";  break;
    case GigCategory::OTHER:     roleStr = "OTHER";     break;
    default:                     roleStr = "?";         break;
    }
    cout << "  [" << gig.getGigID() << "] "
        << gig.getTitle()
        << "  | " << roleStr
        << " | Rs " << gig.getPrice()
        << " | " << (gig.getIsActive() ? "active" : "inactive")
        << "\n";
}

void CLIApp::printGigDetail(const Gig& gig) {
    string roleStr;
    switch (gig.getCategory()) {
    case GigCategory::CODING:    roleStr = "CODING";    break;
    case GigCategory::DESIGN:    roleStr = "DESIGN";    break;
    case GigCategory::WRITING:   roleStr = "WRITING";   break;
    case GigCategory::MARKETING: roleStr = "MARKETING"; break;
    case GigCategory::TUTORING:  roleStr = "TUTORING";  break;
    case GigCategory::OTHER:     roleStr = "OTHER";     break;
    default:                     roleStr = "?";         break;
    }
    cout << "Gig ID:      " << gig.getGigID() << "\n";
    cout << "Title:       " << gig.getTitle() << "\n";
    cout << "Owner ID:    " << gig.getOwnerID() << "\n";
    cout << "Category:    " << roleStr << "\n";
    cout << "Price:       Rs " << gig.getPrice() << "\n";
    cout << "Active:      " << (gig.getIsActive() ? "yes" : "no") << "\n";
    cout << "Created at:  " << gig.getCreatedAt() << "\n";
    cout << "Description:\n  " << gig.getDescription() << "\n";
}

bool CLIApp::readCategory(GigCategory& out) {
    cout << "Category:\n";
    cout << "  1. Coding\n";
    cout << "  2. Design\n";
    cout << "  3. Writing\n";
    cout << "  4. Marketing\n";
    cout << "  5. Tutoring\n";
    cout << "  6. Other\n";
    int c = readMenuChoice();
    switch (c) {
    case 1: out = GigCategory::CODING;    return true;
    case 2: out = GigCategory::DESIGN;    return true;
    case 3: out = GigCategory::WRITING;   return true;
    case 4: out = GigCategory::MARKETING; return true;
    case 5: out = GigCategory::TUTORING;  return true;
    case 6: out = GigCategory::OTHER;     return true;
    default:
        cout << "Invalid category.\n";
        return false;
    }
}


void CLIApp::showLoggedOutMenu() {
    cout << "\n";
    cout << "1. Register\n";
    cout << "2. Login\n";
    cout << "3. Exit\n";

    int choice = readMenuChoice();
    switch (choice) {
    case 1: doRegister(); break;
    case 2: doLogin();    break;
    case 3: running_ = false; break;
    default: cout << "Invalid choice. Try again.\n"; break;
    }
}

void CLIApp::doRegister() {
    printHeader("Register");

    try {
        string name = readLine("Name:     ");
        string email = readLine("Email:    ");
        string pwd = readLine("Password: ");

        cout << "Role:\n";
        cout << "  1. Client\n";
        cout << "  2. Freelancer\n";
        cout << "  3. Admin\n";
        int rc = readMenuChoice();

        UserRole role;
        switch (rc) {
        case 1: role = UserRole::CLIENT;     break;
        case 2: role = UserRole::FREELANCER; break;
        case 3: role = UserRole::ADMIN;      break;
        default:
            cout << "Invalid role.\n";
            return;
        }

        User* u = userManager_.registerUser(name, email, pwd, role);
        cout << "\nRegistered successfully. Welcome, "
            << u->getName() << ".\n";
    }
    catch (const SkillBridgeException& e) {
        cout << "\nError: " << e.what() << "\n";
    }
}

void CLIApp::doLogin() {
    printHeader("Login");

    try {
        string email = readLine("Email:    ");
        string pwd = readLine("Password: ");

        User* u = userManager_.login(email, pwd);
        cout << "\nLogged in as " << u->getName() << ".\n";
    }
    catch (const SkillBridgeException& e) {
        cout << "\nError: " << e.what() << "\n";
    }
}



void CLIApp::showLoggedInMenu() {
    User* u = userManager_.getCurrentUser();
    cout << "\n=== Welcome, " << u->getName() << " ===\n";
    cout << "1. My Profile\n";
    cout << "2. Gigs\n";
    cout << "3. Logout\n";
    cout << "4. Exit\n";

    int choice = readMenuChoice();
    switch (choice) {
    case 1: showProfileMenu(); break;
    case 2: showGigMenu();     break;
    case 3: doLogout();        break;
    case 4: running_ = false;  break;
    default: cout << "Invalid choice. Try again.\n"; break;
    }
}



void CLIApp::showProfileMenu() {
    while (userManager_.isLoggedIn()) {
        cout << "\n-- My Profile --\n";
        cout << "1. View profile\n";
        cout << "2. Update name\n";
        cout << "3. Change password\n";
        cout << "4. Delete account\n";
        cout << "5. Back\n";
        int choice = readMenuChoice();
        switch (choice) {
        case 1: doViewProfile();    break;
        case 2: doUpdateName();     break;
        case 3: doChangePassword(); break;
        case 4: doDeleteAccount();  break;
        case 5: return;
        default: cout << "Invalid choice. Try again.\n"; break;
        }
    }
}

void CLIApp::doViewProfile() {
    printHeader("Profile");

    User* u = userManager_.getCurrentUser();
    if (!u) {
        cout << "Not logged in.\n";
        return;
    }

    string roleStr;
    switch (u->getRole()) {
    case UserRole::CLIENT:     roleStr = "Client";     break;
    case UserRole::FREELANCER: roleStr = "Freelancer"; break;
    case UserRole::ADMIN:      roleStr = "Admin";      break;
    default:                   roleStr = "Unknown";    break;
    }

    cout << "User ID:   " << u->getUserID() << "\n";
    cout << "Name:      " << u->getName() << "\n";
    cout << "Email:     " << u->getEmail() << "\n";
    cout << "Role:      " << roleStr << "\n";
    cout << "Balance:   " << u->getBalance() << "\n";
    cout << "Joined:    " << u->getCreatedAt() << "\n";
}

void CLIApp::doUpdateName() {
    printHeader("Update Name");
    try {
        string newName = readLine("New name: ");
        if (userManager_.updateProfile(newName)) {
            cout << "\nName updated successfully.\n";
        }
        else {
            cout << "\nUpdate failed.\n";
        }
    }
    catch (const SkillBridgeException& e) {
        cout << "\nError: " << e.what() << "\n";
    }
}

void CLIApp::doChangePassword() {
    printHeader("Change Password");
    try {
        string oldPwd = readLine("Current password: ");
        string newPwd = readLine("New password:     ");
        if (userManager_.changePassword(oldPwd, newPwd)) {
            cout << "\nPassword changed successfully.\n";
        }
        else {
            cout << "\nPassword change failed.\n";
        }
    }
    catch (const SkillBridgeException& e) {
        cout << "\nError: " << e.what() << "\n";
    }
}

void CLIApp::doDeleteAccount() {
    printHeader("Delete Account");
    try {
        string confirm = readLine(
            "Type your password to confirm deletion: ");
        if (userManager_.deleteAccount(confirm)) {
            cout << "\nAccount deleted. Goodbye.\n";
        }
        else {
            cout << "\nDelete failed.\n";
        }
    }
    catch (const SkillBridgeException& e) {
        cout << "\nError: " << e.what() << "\n";
    }
}

void CLIApp::doLogout() {
    userManager_.logout();
    cout << "\nLogged out.\n";
}



void CLIApp::showGigMenu() {
    User* u = userManager_.getCurrentUser();
    if (!u) return;

    switch (u->getRole()) {
    case UserRole::FREELANCER: showFreelancerGigMenu(); break;
    case UserRole::CLIENT:     showClientGigMenu();     break;
    case UserRole::ADMIN:      showAdminGigMenu();      break;
    default: cout << "Unknown role.\n"; break;
    }
}

void CLIApp::showFreelancerGigMenu() {
    while (userManager_.isLoggedIn()) {
        cout << "\n-- Gigs (Freelancer) --\n";
        cout << "1. Browse all active gigs\n";
        cout << "2. My gigs\n";
        cout << "3. Create a new gig\n";
        cout << "4. Back\n";
        int choice = readMenuChoice();
        switch (choice) {
        case 1: doBrowseGigs(); break;
        case 2: doMyGigs();     break;
        case 3: doCreateGig();  break;
        case 4: return;
        default: cout << "Invalid choice. Try again.\n"; break;
        }
    }
}

void CLIApp::showClientGigMenu() {
    while (userManager_.isLoggedIn()) {
        cout << "\n-- Gigs (Client) --\n";
        cout << "1. Browse all active gigs\n";
        cout << "2. Autocomplete suggestions (preview)\n";
        cout << "3. Back\n";
        int choice = readMenuChoice();
        switch (choice) {
        case 1: doBrowseGigs();         break;
        case 2: doAutocompleteDemo();   break;
        case 3: return;
        default: cout << "Invalid choice. Try again.\n"; break;
        }
    }
}

void CLIApp::showAdminGigMenu() {
    while (userManager_.isLoggedIn()) {
        cout << "\n-- Gigs (Admin) --\n";
        cout << "1. Browse all active gigs\n";
        cout << "2. View ALL gigs (including inactive)\n";
        cout << "3. Hard-delete a gig\n";
        cout << "4. Back\n";
        int choice = readMenuChoice();
        switch (choice) {
        case 1: doBrowseGigs();    break;
        case 2: doViewAllGigs();   break;
        case 3: doHardDeleteGig(); break;
        case 4: return;
        default: cout << "Invalid choice. Try again.\n"; break;
        }
    }
}


void CLIApp::doBrowseGigs() {
    printHeader("Browse Gigs");
    cout << "Enter a search query, or leave empty to list all active gigs.\n";
    string q = readLine("Query: ");

    try {
        DataList<Gig> results;
        if (q.empty()) {
            results = gigManager_.findAllActiveGigs();
        }
        else {
           
            results = gigManager_.searchGigs(q, 1000);
        }

        if (results.size() == 0) {
            cout << "\nNo gigs found.\n";
            return;
        }

       
        const int PAGE = 10;
        int shown = 0;
        while (shown < results.size()) {
            cout << "\n";
            int end = shown + PAGE;
            if (end > results.size()) end = results.size();
            for (int i = shown; i < end; ++i) {
                printGigSummary(results.get(i));
            }
            cout << "(showing " << (shown + 1) << "-" << end
                << " of " << results.size() << ")\n";
            shown = end;

            if (shown < results.size()) {
                string more = readLine("Show next page? (y/n): ");
                if (more != "y" && more != "Y") break;
            }
        }

       
        string view = readLine("View gig by ID? (enter ID, or empty to skip): ");
        if (!view.empty()) {
            try {
                int id = stoi(view);
                Gig g = gigManager_.findGigByID(id);
                cout << "\n";
                printGigDetail(g);
            }
            catch (const invalid_argument&) {
                cout << "Invalid ID.\n";
            }
            catch (const SkillBridgeException& e) {
                cout << "Error: " << e.what() << "\n";
            }
        }
    }
    catch (const SkillBridgeException& e) {
        cout << "\nError: " << e.what() << "\n";
    }
}

void CLIApp::doSearchGigs() {
   
}



void CLIApp::doMyGigs() {
    printHeader("My Gigs");
    User* u = userManager_.getCurrentUser();
    if (!u) return;

    try {
        DataList<Gig> mine = gigManager_.findGigsByOwner(u->getUserID());
        if (mine.size() == 0) {
            cout << "You have no gigs yet.\n";
            return;
        }

        cout << "\nYour gigs (active and inactive):\n";
        for (int i = 0; i < mine.size(); ++i) {
            printGigSummary(mine.get(i));
        }

        cout << "\n1. Edit a gig\n";
        cout << "2. Deactivate a gig\n";
        cout << "3. Back\n";
        int choice = readMenuChoice();

        if (choice == 3) return;

        int gigID = readInt("Gig ID: ");
        if (gigID <= 0) {
            cout << "Invalid ID.\n";
            return;
        }

        if (choice == 1) {
            doEditMyGig(gigID);
        }
        else if (choice == 2) {
            doDeactivateMyGig(gigID);
        }
        else {
            cout << "Invalid choice.\n";
        }
    }
    catch (const SkillBridgeException& e) {
        cout << "\nError: " << e.what() << "\n";
    }
}

void CLIApp::doCreateGig() {
    printHeader("Create Gig");

    User* u = userManager_.getCurrentUser();
    if (!u) return;

    try {
        string title = readLine("Title (3-100 chars):       ");
        string desc = readLine("Description (10-2000):     ");
        double price = readDouble("Price (positive, < 1M):    ");
        GigCategory cat;
        if (!readCategory(cat)) return;

        Gig g = gigManager_.createGig(
            u->getUserID(), title, desc, price, cat);

        cout << "\nGig created successfully.\n";
        printGigDetail(g);
    }
    catch (const SkillBridgeException& e) {
        cout << "\nError: " << e.what() << "\n";
    }
}

void CLIApp::doEditMyGig(int gigID) {
    printHeader("Edit Gig");

    User* u = userManager_.getCurrentUser();
    if (!u) return;

    try {
       
        Gig current = gigManager_.findGigByID(gigID);
        cout << "\nCurrent values:\n";
        printGigDetail(current);

        cout << "\nEnter new values (validated by the manager):\n";
        string title = readLine("New title:        ");
        string desc = readLine("New description:  ");
        double price = readDouble("New price:        ");
        GigCategory cat;
        if (!readCategory(cat)) return;

        gigManager_.updateGig(u->getUserID(), gigID, title, desc, price, cat);
        cout << "\nGig updated successfully.\n";
    }
    catch (const SkillBridgeException& e) {
        cout << "\nError: " << e.what() << "\n";
    }
}

void CLIApp::doDeactivateMyGig(int gigID) {
    printHeader("Deactivate Gig");

    User* u = userManager_.getCurrentUser();
    if (!u) return;

    try {
        gigManager_.deactivateGig(u->getUserID(), gigID);
        cout << "\nGig deactivated. It will no longer appear in search.\n";
    }
    catch (const SkillBridgeException& e) {
        cout << "\nError: " << e.what() << "\n";
    }
}



void CLIApp::doAutocompleteDemo() {
    printHeader("Autocomplete");

    cout << "Type a prefix to see top matching tokens from the gig corpus.\n";
    string prefix = readLine("Prefix: ");

    try {
        DataList<string> sugg = gigManager_.autocompleteSuggestions(prefix, 10);
        if (sugg.size() == 0) {
            cout << "\nNo suggestions for that prefix.\n";
            return;
        }
        cout << "\nSuggestions (ranked by frequency):\n";
        for (int i = 0; i < sugg.size(); ++i) {
            cout << "  " << (i + 1) << ". " << sugg.get(i) << "\n";
        }
    }
    catch (const SkillBridgeException& e) {
        cout << "\nError: " << e.what() << "\n";
    }
}



void CLIApp::doViewAllGigs() {
    printHeader("All Gigs (active and inactive)");
    User* u = userManager_.getCurrentUser();
    if (!u) return;

    try {
        DataList<Gig> all = gigManager_.findAllGigs(u->getUserID());
        if (all.size() == 0) {
            cout << "No gigs in the system.\n";
            return;
        }
        for (int i = 0; i < all.size(); ++i) {
            printGigSummary(all.get(i));
        }
        cout << "\n(total: " << all.size() << ")\n";
    }
    catch (const SkillBridgeException& e) {
        cout << "\nError: " << e.what() << "\n";
    }
}

void CLIApp::doHardDeleteGig() {
    printHeader("Hard-Delete Gig");
    cout << "Warning: this permanently removes the gig.\n";

    User* u = userManager_.getCurrentUser();
    if (!u) return;

    try {
        int id = readInt("Gig ID to delete: ");
        if (id <= 0) {
            cout << "Invalid ID.\n";
            return;
        }
        string confirm = readLine("Type 'DELETE' to confirm: ");
        if (confirm != "DELETE") {
            cout << "Cancelled.\n";
            return;
        }
        gigManager_.deleteGig(u->getUserID(), id);
        cout << "\nGig deleted.\n";
    }
    catch (const SkillBridgeException& e) {
        cout << "\nError: " << e.what() << "\n";
    }
}