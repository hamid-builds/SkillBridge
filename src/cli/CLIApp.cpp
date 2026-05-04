#include "cli/CLIApp.h"
#include "managers/UserManager.h"
#include "managers/GigManager.h"
#include "managers/OrderManager.h"
#include "managers/CommandHistory.h"
#include "managers/PlaceOrderCommand.h"
#include "managers/CancelOrderCommand.h"
#include "managers/UpdateStatusCommand.h"
#include "managers/MessageManager.h"
#include "managers/ReviewManager.h"
#include "managers/SkillGraphManager.h"
#include "core/User.h"
#include "core/UserRole.h"
#include "core/Gig.h"
#include "core/GigCategory.h"
#include "core/Order.h"
#include "core/OrderStatus.h"
#include "core/Review.h"
#include "core/Endorsement.h"
#include "core/Exceptions.h"
#include "utils/DataList.h"

#include <iostream>
#include <iomanip>
#include <limits>
#include <string>
#include <stdexcept>
#include <cstdio>

using namespace std;

CLIApp::CLIApp(UserManager& userManager, GigManager& gigManager, OrderManager& orderManager, CommandHistory& history, MessageManager& messageManager, ReviewManager& reviewManager, SkillGraphManager& skillGraphManager)
    : userManager_(userManager), gigManager_(gigManager), orderManager_(orderManager), history_(history), messageManager_(messageManager), reviewManager_(reviewManager), skillGraphManager_(skillGraphManager), running_(true)
{
}

void CLIApp::run() {
    cout << "\n";
    cout << "========================================\n";
    cout << "    Welcome to SkillBridge (CLI)\n";
    cout << "========================================\n";

    while (running_) {
        if (userManager_.isLoggedIn())
        {
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
    if (!(cin >> choice)) 
    {
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
    try { return stoi(line); }
    catch (...) { return -1; }
}

double CLIApp::readDouble(const string& prompt) {
    cout << prompt;
    string line;
    getline(cin, line);
    try { return stod(line); }
    catch (...) { return -1.0; }
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

static std::string formatRs(double amount) {
    char buf[64];
    std::snprintf(buf, sizeof(buf), "Rs %.2f", amount);
    return std::string(buf);
}
string CLIApp::roleLabel(const User& u) const {
    switch (u.getRole()) {
    case UserRole::CLIENT:     return "Client";
    case UserRole::FREELANCER: return "Freelancer";
    case UserRole::ADMIN:      return "Admin";
    }
    return "?";
}
void CLIApp::printGigSummary(const Gig& gig) {
    string roleStr;
    switch (gig.getCategory()) {
    case GigCategory::CODING:
        roleStr = "CODING";  
        break;
    case GigCategory::DESIGN:
        roleStr = "DESIGN";  
        break;
    case GigCategory::WRITING:   
        roleStr = "WRITING"; 
        break;
    case GigCategory::MARKETING: 
        roleStr = "MARKETING"; 
        break;
    case GigCategory::TUTORING:  
        roleStr = "TUTORING";  
        break;
    case GigCategory::OTHER:     
        roleStr = "OTHER";   
        break;
    default:                     
        roleStr = "?";         
        break;
    }
    cout << "  [" << gig.getGigID() << "] "  << gig.getTitle()  << "  | " << roleStr  << " | " << formatRs(gig.getPrice())
        << " | " << (gig.getIsActive() ? "active" : "inactive") << "\n";
}

void CLIApp::printGigDetail(const Gig& gig) {
    string roleStr;
    switch (gig.getCategory()) {
    case GigCategory::CODING:    roleStr = "CODING";  
        break;
    case GigCategory::DESIGN:    roleStr = "DESIGN";  
        break;
    case GigCategory::WRITING:   roleStr = "WRITING"; 
        break;
    case GigCategory::MARKETING: roleStr = "MARKETING";
        break;
    case GigCategory::TUTORING:  roleStr = "TUTORING"; 
        break;
    case GigCategory::OTHER:     roleStr = "OTHER";  
        break;
    default:                     roleStr = "?";         break;
    }
    cout << "Gig ID:      " << gig.getGigID() << "\n";
    cout << "Title:       " << gig.getTitle() << "\n";
    cout << "Owner ID:    " << gig.getOwnerID() << "\n";
    cout << "Category:    " << roleStr << "\n";
    cout << "Price:       " << formatRs(gig.getPrice()) << "\n";
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
    case 1: out = GigCategory::CODING;  
        return true;
    case 2: out = GigCategory::DESIGN;  
        return true;
    case 3: out = GigCategory::WRITING; 
        return true;
    case 4: out = GigCategory::MARKETING;
        return true;
    case 5: out = GigCategory::TUTORING; 
        return true;
    case 6: out = GigCategory::OTHER;    
        return true;
    default:
        cout << "Invalid category.\n";
        return false;
    }
}



void CLIApp::printOrderSummary(const Order& o) {
    cout << "  [#" << o.getOrderID() << "] "
        << "gig=" << o.getGigID()
        << " | buyer=" << o.getBuyerID()
        << " | seller=" << o.getSellerID()
        << " | " << formatRs(o.getAmount())
        << " | " << orderStatusToString(o.getStatus())
        << " | due " << o.getDeadline()
        << "\n";
}

void CLIApp::printOrderDetail(const Order& o) {
    cout << "Order ID:      #" << o.getOrderID() << "\n";
    cout << "Gig ID:        " << o.getGigID() << "\n";
    cout << "Buyer ID:      " << o.getBuyerID() << "\n";
    cout << "Seller ID:     " << o.getSellerID() << "\n";
    cout << "Amount:        " << formatRs(o.getAmount()) << "\n";
    cout << "Status:        " << orderStatusToString(o.getStatus()) << "\n";
    cout << "Placed at:     " << o.getPlacedAt() << "\n";
    cout << "Deadline:      " << o.getDeadline() << "\n";
    string completed = o.getCompletedAt();
    cout << "Completed at:  " << (completed.empty() ? "(not completed)"
        : completed) << "\n";
}

bool CLIApp::readTargetStatus(OrderStatus& out) {
    cout << "Target status:\n";
    cout << "  1. IN_PROGRESS (accept and start work)\n";
    cout << "  2. COMPLETED (mark work done; credits seller)\n";
    cout << "  3. CANCELLED (cancel; refunds buyer)\n";
    int c = readMenuChoice();
    switch (c) {
    case 1: out = OrderStatus::IN_PROGRESS;
        return true;
    case 2: out = OrderStatus::COMPLETED;  
        return true;
    case 3: out = OrderStatus::CANCELLED;   return true;
    default:
        cout << "Invalid status.\n";
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
    case 1: doRegister();
        break;
    case 2: doLogin();   
        break;
    case 3: running_ = false;
        break;
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
        case 1: role = UserRole::CLIENT;    
            break;
        case 2: role = UserRole::FREELANCER;
            break;
        case 3: role = UserRole::ADMIN;     
            break;
        default: cout << "Invalid role.\n"; return;
        }

        User* u = userManager_.registerUser(name, email, pwd, role);
        cout << "\nRegistered successfully. Welcome, " << u->getName() << ".\n";
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

   
    string undoDesc = history_.peekUndoDescription();
    string redoDesc = history_.peekRedoDescription();

    bool isAdmin = (u->getRole() == UserRole::ADMIN);

    cout << "1. My Profile\n";
    cout << "2. Gigs\n";
    cout << "3. Orders\n";
    cout << "4. Messages\n";
    cout << "5. Reviews\n";
    cout << "6. Endorsements\n";

    int next = 7;
    int adminChoice = -1;
    if (isAdmin) {
        adminChoice = next++;
        cout << adminChoice << ". Admin Panel\n";
    }
    int undoChoice = next++;
    cout << undoChoice << ". Undo";
    if (!undoDesc.empty()) cout << "  (" << undoDesc << ")";
    cout << "\n";
    int redoChoice = next++;
    cout << redoChoice << ". Redo";
    if (!redoDesc.empty()) cout << "  (" << redoDesc << ")";
    cout << "\n";
    int logoutChoice = next++;
    cout << logoutChoice << ". Logout\n";
    int exitChoice = next++;
    cout << exitChoice << ". Exit\n";
    int choice = readMenuChoice();
    if (choice == 1)                  
        showProfileMenu();
    else if (choice == 2)             
        showGigMenu();
    else if (choice == 3)             
        showOrderMenu();
    else if (choice == 4)             
        showMessageMenu();
    else if (choice == 5)             
        showReviewMenu();
    else if (choice == 6)             
        showEndorsementMenu();
    else if (isAdmin && choice == adminChoice) 
        showAdminPanelMenu();
    else if (choice == undoChoice)    
        doUndo();
    else if (choice == redoChoice)    
        doRedo();
    else if (choice == logoutChoice)  
        doLogout();
    else if (choice == exitChoice)    
        running_ = false;
    else                              
        cout << "Invalid choice. Try again.\n";
}

void CLIApp::showProfileMenu() {
    while (userManager_.isLoggedIn()) {
        cout << "\n-- My Profile --\n";
        cout << "1. View profile\n";
        cout << "2. Update name\n";
        cout << "3. Change password\n";
        cout << "4. Deposit funds\n";
        cout << "5. Delete account\n";
        cout << "6. Back\n";
        int choice = readMenuChoice();
        switch (choice) {
        case 1: doViewProfile();   
            break;
        case 2: doUpdateName();    
            break;
        case 3: doChangePassword();
            break;
        case 4: doDeposit();       
            break;
        case 5: doDeleteAccount(); 
            break;
        case 6: return;
        default: cout << "Invalid choice. Try again.\n"; break;
        }
    }
}

void CLIApp::doViewProfile() {
    printHeader("Profile");
    User* u = userManager_.getCurrentUser();
    if (!u) { cout << "Not logged in.\n"; return; }

    cout << "User ID:   " << u->getUserID() << "\n";
    cout << "Name:      " << u->getName() << "\n";
    cout << "Email:     " << u->getEmail() << "\n";
    cout << "Role:      " << roleLabel(*u) << "\n";
    cout << "Balance:   " << formatRs(u->getBalance()) << "\n";
    cout << "Joined:    " << u->getCreatedAt() << "\n";
}

void CLIApp::doUpdateName() {
    printHeader("Update Name");
    try {
        string newName = readLine("New name: ");
        if (userManager_.updateProfile(newName))
            cout << "\nName updated successfully.\n";
        else
            cout << "\nUpdate failed.\n";
    }
    catch (const SkillBridgeException& e) { cout << "\nError: " << e.what() << "\n"; }
}

void CLIApp::doChangePassword() {
    printHeader("Change Password");
    try {
        string oldPwd = readLine("Current password: ");
        string newPwd = readLine("New password:     ");
        if (userManager_.changePassword(oldPwd, newPwd))
            cout << "\nPassword changed successfully.\n";
        else
            cout << "\nPassword change failed.\n";
    }
    catch (const SkillBridgeException& e) { cout << "\nError: " << e.what() << "\n"; }
}

void CLIApp::doDeposit() {
    printHeader("Deposit Funds");
    User* u = userManager_.getCurrentUser();
    if (!u) return;

    cout << "Current balance: " << formatRs(u->getBalance()) << "\n";

    
    string raw = readLine("Amount to deposit (Rs), or empty to cancel: ");
    if (raw.empty()) {
        cout << "Cancelled.\n";
        return;
    }

    double amount;
    try {
        amount = stod(raw);
    }
    catch (...) {
        cout << "Invalid amount.\n";
        return;
    }

    try {
        if (userManager_.depositToBalance(amount)) {
            cout << "\nDeposit successful. New balance: "
                << formatRs(u->getBalance()) << "\n";
        }
    }
    catch (const SkillBridgeException& e) {
        cout << "\nError: " << e.what() << "\n";
    }
}


void CLIApp::doDeleteAccount() {
    printHeader("Delete Account");
    try {
        string confirm = readLine("Type your password to confirm deletion: ");
        if (userManager_.deleteAccount(confirm))
            cout << "\nAccount deleted. Goodbye.\n";
        else
            cout << "\nDelete failed.\n";
    }
    catch (const SkillBridgeException& e) { cout << "\nError: " << e.what() << "\n"; }
}

void CLIApp::doLogout() {
    
    history_.clear();
    messageManager_.clearCache();
    skillGraphManager_.clearGraph();
    userManager_.logout();
    cout << "\nLogged out.\n";
}


void CLIApp::showGigMenu() {
    User* u = userManager_.getCurrentUser();
    if (!u) return;
    switch (u->getRole()) {
    case UserRole::FREELANCER: showFreelancerGigMenu(); 
        break;
    case UserRole::CLIENT:     showClientGigMenu();    
        break;
    case UserRole::ADMIN:      showAdminGigMenu();     
        break;
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
        case 1: doBrowseGigs(); 
            break;
        case 2: doMyGigs();    
            break;
        case 3: doCreateGig(); 
            break;
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
        case 1: doBrowseGigs();       
            break;
        case 2: doAutocompleteDemo(); 
            break;
        case 3: return;
        default: cout << "Invalid choice. Try again.\n";
            break;
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
        case 1: doBrowseGigs();   
            break;
        case 2: doViewAllGigs();  
            break;
        case 3: doHardDeleteGig();
            break;
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
        if (q.empty()) results = gigManager_.findAllActiveGigs();
        else           results = gigManager_.searchGigs(q, 1000);

        if (results.size() == 0) { cout << "\nNo gigs found.\n"; return; }

        const int PAGE = 10;
        int shown = 0;
        while (shown < results.size()) {
            cout << "\n";
            int end = shown + PAGE;
            if (end > results.size()) end = results.size();
            for (int i = shown; i < end; ++i) printGigSummary(results.get(i));
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
            catch (const invalid_argument&) { cout << "Invalid ID.\n"; }
            catch (const SkillBridgeException& e) { cout << "Error: " << e.what() << "\n"; }
        }
    }
    catch (const SkillBridgeException& e) { cout << "\nError: " << e.what() << "\n"; }
}

void CLIApp::doSearchGigs() {}

void CLIApp::doMyGigs() {
    printHeader("My Gigs");
    User* u = userManager_.getCurrentUser();
    if (!u) return;
    try {
        DataList<Gig> mine = gigManager_.findGigsByOwner(u->getUserID());
        if (mine.size() == 0) { cout << "You have no gigs yet.\n"; return; }
        cout << "\nYour gigs (active and inactive):\n";
        for (int i = 0; i < mine.size(); ++i) printGigSummary(mine.get(i));
        cout << "\n1. Edit a gig\n2. Deactivate a gig\n3. Back\n";
        int choice = readMenuChoice();
        if (choice == 3) return;
        int gigID = readInt("Gig ID: ");
        if (gigID <= 0) { cout << "Invalid ID.\n"; return; }
        if (choice == 1) doEditMyGig(gigID);
        else if (choice == 2) doDeactivateMyGig(gigID);
        else cout << "Invalid choice.\n";
    }
    catch (const SkillBridgeException& e) { cout << "\nError: " << e.what() << "\n"; }
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
        Gig g = gigManager_.createGig(u->getUserID(), title, desc, price, cat);
        cout << "\nGig created successfully.\n";
        printGigDetail(g);
    }
    catch (const SkillBridgeException& e) { cout << "\nError: " << e.what() << "\n"; }
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
    catch (const SkillBridgeException& e) { cout << "\nError: " << e.what() << "\n"; }
}

void CLIApp::doDeactivateMyGig(int gigID) {
    printHeader("Deactivate Gig");
    User* u = userManager_.getCurrentUser();
    if (!u) return;
    try {
        gigManager_.deactivateGig(u->getUserID(), gigID);
        cout << "\nGig deactivated. It will no longer appear in search.\n";
    }
    catch (const SkillBridgeException& e) { cout << "\nError: " << e.what() << "\n"; }
}

void CLIApp::doAutocompleteDemo() {
    printHeader("Autocomplete");
    cout << "Type a prefix to see top matching tokens from the gig corpus.\n";
    string prefix = readLine("Prefix: ");
    try {
        DataList<string> sugg = gigManager_.autocompleteSuggestions(prefix, 10);
        if (sugg.size() == 0) { cout << "\nNo suggestions for that prefix.\n"; return; }
        cout << "\nSuggestions (ranked by frequency):\n";
        for (int i = 0; i < sugg.size(); ++i)
            cout << "  " << (i + 1) << ". " << sugg.get(i) << "\n";
    }
    catch (const SkillBridgeException& e) { cout << "\nError: " << e.what() << "\n"; }
}

void CLIApp::doViewAllGigs() {
    printHeader("All Gigs (active and inactive)");
    User* u = userManager_.getCurrentUser();
    if (!u) return;
    try {
        DataList<Gig> all = gigManager_.findAllGigs(u->getUserID());
        if (all.size() == 0) { cout << "No gigs in the system.\n"; return; }
        for (int i = 0; i < all.size(); ++i) printGigSummary(all.get(i));
        cout << "\n(total: " << all.size() << ")\n";
    }
    catch (const SkillBridgeException& e) { cout << "\nError: " << e.what() << "\n"; }
}

void CLIApp::doHardDeleteGig() {
    printHeader("Hard-Delete Gig");
    cout << "Warning: this permanently removes the gig.\n";
    User* u = userManager_.getCurrentUser();
    if (!u) return;
    try {
        int id = readInt("Gig ID to delete: ");
        if (id <= 0) { cout << "Invalid ID.\n"; return; }
        string confirm = readLine("Type 'DELETE' to confirm: ");
        if (confirm != "DELETE") { cout << "Cancelled.\n"; return; }
        gigManager_.deleteGig(u->getUserID(), id);
        cout << "\nGig deleted.\n";
    }
    catch (const SkillBridgeException& e) { cout << "\nError: " << e.what() << "\n"; }
}


void CLIApp::showOrderMenu() {
    User* u = userManager_.getCurrentUser();
    if (!u) return;
    switch (u->getRole()) {
    case UserRole::CLIENT:     showClientOrderMenu();    
        break;
    case UserRole::FREELANCER: showFreelancerOrderMenu(); 
        break;
    case UserRole::ADMIN: showAdminOrderMenu();     
        break;
    default: cout << "Unknown role.\n"; break;
    }
}

void CLIApp::showClientOrderMenu() {
    while (userManager_.isLoggedIn()) {
        cout << "\n-- Orders (Client) --\n";
        cout << "1. Place a new order\n";
        cout << "2. View my orders\n";
        cout << "3. View urgent orders (by deadline)\n";
        cout << "4. Cancel an order\n";
        cout << "5. Back\n";
        int choice = readMenuChoice();
        switch (choice) {
        case 1: doPlaceOrder();         
            break;
        case 2: doViewMyOrdersAsBuyer(); 
            break;
        case 3: doViewUrgentOrders();    
            break;
        case 4: doCancelOrder();        
            break;
        case 5: return;
        default: cout << "Invalid choice. Try again.\n"; break;
        }
    }
}

void CLIApp::showFreelancerOrderMenu() {
    while (userManager_.isLoggedIn()) {
        cout << "\n-- Orders (Freelancer) --\n";
        cout << "1. View incoming orders\n";
        cout << "2. View urgent orders (by deadline)\n";
        cout << "3. Update order status\n";
        cout << "4. Cancel an order\n";
        cout << "5. Back\n";
        int choice = readMenuChoice();
        switch (choice) {
        case 1: doViewIncomingOrders();
            break;
        case 2: doViewUrgentOrders();   
            break;
        case 3: doUpdateOrderStatus(); 
            break;
        case 4: doCancelOrder();       
            break;
        case 5: return;
        default: cout << "Invalid choice. Try again.\n"; break;
        }
    }
}

void CLIApp::showAdminOrderMenu() {
    while (userManager_.isLoggedIn()) {
        cout << "\n-- Orders (Admin) --\n";
        cout << "1. View ALL orders\n";
        cout << "2. View urgent orders (by deadline)\n";
        cout << "3. Update order status (any order)\n";
        cout << "4. Cancel an order (any order)\n";
        cout << "5. Back\n";
        int choice = readMenuChoice();
        switch (choice) {
        case 1: doViewAllOrders();    
            break;
        case 2: doViewUrgentOrders(); 
            break;
        case 3: doUpdateOrderStatus();
            break;
        case 4: doCancelOrder();     
            break;
        case 5: return;
        default: cout << "Invalid choice. Try again.\n"; break;
        }
    }
}


void CLIApp::doPlaceOrder() {
    printHeader("Place Order");
    User* u = userManager_.getCurrentUser();
    if (!u) return;
    try {
        int gigID = readInt("Gig ID to order: ");
        if (gigID <= 0) { cout << "Invalid gig ID.\n"; return; }

        
        Gig g = gigManager_.findGigByID(gigID);
        cout << "\n";
        printGigDetail(g);
        cout << "\n";

        string deadline = readLine("Deadline (YYYY-MM-DD): ");

        
        history_.recordAndExecute(
            new PlaceOrderCommand(orderManager_, u->getUserID(),
                gigID, deadline));

        cout << "\nOrder placed successfully.\n";
    }
    catch (const SkillBridgeException& e) {
        cout << "\nError: " << e.what() << "\n";
    }
}

void CLIApp::doViewMyOrdersAsBuyer() {
    printHeader("My Orders (as buyer)");
    User* u = userManager_.getCurrentUser();
    if (!u) return;
    try {
        DataList<Order> mine = orderManager_.findByBuyer(u->getUserID());
        if (mine.size() == 0) { cout << "You have placed no orders.\n"; return; }
        for (int i = 0; i < mine.size(); ++i) printOrderSummary(mine.get(i));
        cout << "\n(total: " << mine.size() << ")\n";

        string view = readLine("View order in detail? (enter ID, or empty to skip): ");
        if (!view.empty()) {
            try {
                int id = stoi(view);
                Order o = orderManager_.findOrderByID(u->getUserID(), id);
                cout << "\n";
                printOrderDetail(o);
            }
            catch (const invalid_argument&) { cout << "Invalid ID.\n"; }
            catch (const SkillBridgeException& e) { cout << "Error: " << e.what() << "\n"; }
        }
    }
    catch (const SkillBridgeException& e) { cout << "\nError: " << e.what() << "\n"; }
}

void CLIApp::doViewIncomingOrders() {
    printHeader("Incoming Orders (as seller)");
    User* u = userManager_.getCurrentUser();
    if (!u) return;
    try {
        DataList<Order> mine = orderManager_.findBySeller(u->getUserID());
        if (mine.size() == 0) { cout << "No incoming orders yet.\n"; return; }
        for (int i = 0; i < mine.size(); ++i) printOrderSummary(mine.get(i));
        cout << "\n(total: " << mine.size() << ")\n";
    }
    catch (const SkillBridgeException& e) { cout << "\nError: " << e.what() << "\n"; }
}

void CLIApp::doViewAllOrders() {
    printHeader("All Orders (Admin)");
    User* u = userManager_.getCurrentUser();
    if (!u) return;
    try {
        DataList<Order> all = orderManager_.findAll(u->getUserID());
        if (all.size() == 0) { cout << "No orders in the system.\n"; return; }
        for (int i = 0; i < all.size(); ++i) printOrderSummary(all.get(i));
        cout << "\n(total: " << all.size() << ")\n";
    }
    catch (const SkillBridgeException& e) { cout << "\nError: " << e.what() << "\n"; }
}

void CLIApp::doCancelOrder() {
    printHeader("Cancel Order");
    User* u = userManager_.getCurrentUser();
    if (!u) return;
    try {
        int id = readInt("Order ID to cancel: ");
        if (id <= 0) { cout << "Invalid ID.\n"; return; }

        history_.recordAndExecute(
            new CancelOrderCommand(orderManager_, u->getUserID(), id));

        cout << "\nOrder cancelled and refunded.\n";
    }
    catch (const SkillBridgeException& e) {
        cout << "\nError: " << e.what() << "\n";
    }
}

void CLIApp::doUpdateOrderStatus() {
    printHeader("Update Order Status");
    User* u = userManager_.getCurrentUser();
    if (!u) return;
    try {
        int id = readInt("Order ID: ");
        if (id <= 0) { cout << "Invalid ID.\n"; return; }

        OrderStatus target;
        if (!readTargetStatus(target)) return;

        history_.recordAndExecute(
            new UpdateStatusCommand(orderManager_, u->getUserID(),
                id, target));

        cout << "\nStatus updated.\n";
    }
    catch (const SkillBridgeException& e) {
        cout << "\nError: " << e.what() << "\n";
    }
}

void CLIApp::doViewUrgentOrders() {
    printHeader("Urgent Orders (by deadline)");
    User* u = userManager_.getCurrentUser();
    if (!u) return;

    int k = readInt("How many to show? (e.g. 5): ");
    if (k <= 0) { cout << "Invalid count.\n"; return; }

    try {
        DataList<Order> urgent = orderManager_.findUrgentOrders(
            u->getUserID(), k);

        if (urgent.size() == 0) {
            cout << "No active orders to show.\n";
            return;
        }

        cout << "\nMost urgent first:\n";
        for (int i = 0; i < urgent.size(); ++i) {
            printOrderSummary(urgent.get(i));
        }
        cout << "\n(showing top " << urgent.size() << ")\n";
    }
    catch (const SkillBridgeException& e) {
        cout << "\nError: " << e.what() << "\n";
    }
}

void CLIApp::doUndo() {
    if (!history_.canUndo()) {
        cout << "\nNothing to undo.\n";
        return;
    }
    string desc = history_.peekUndoDescription();
    try {
        history_.undo();
        cout << "\nUndone: " << desc << "\n";
    }
    catch (const SkillBridgeException& e) {
        cout << "\nUndo failed: " << e.what() << "\n";
    }
}

void CLIApp::doRedo() {
    if (!history_.canRedo()) {
        cout << "\nNothing to redo.\n";
        return;
    }
    string desc = history_.peekRedoDescription();
    try {
        history_.redo();
        cout << "\nRedone: " << desc << "\n";
    }
    catch (const SkillBridgeException& e) {
        cout << "\nRedo failed: " << e.what() << "\n";
    }
}
void CLIApp::showMessageMenu() {
    while (userManager_.isLoggedIn()) {
        cout << "\n-- Messages --\n";
        cout << "1. Send a message\n";
        cout << "2. View inbox\n";
        cout << "3. View conversation with a user\n";
        cout << "4. Mark conversation as read\n";
        cout << "5. Back\n";
        int choice = readMenuChoice();
        switch (choice) {
        case 1: doSendMessage();
            break;
        case 2: doViewInbox();            
            break;
        case 3: doViewConversation();     
            break;
        case 4: doMarkConversationRead(); 
            break;
        case 5: 
            return;
        default: cout << "Invalid choice. Try again.\n"; 
            break;
        }
    }
}

void CLIApp::doSendMessage() {
    printHeader("Send Message");
    User* u = userManager_.getCurrentUser();
    if (!u) return;
    try {
        int receiverID = readInt("Receiver user ID: ");
        if (receiverID <= 0) {
            cout << "Invalid user ID.\n";
            return;
        }

        string text = readLine("Message: ");
        if (text.empty()) {
            cout << "Empty message; cancelled.\n";
            return;
        }

        messageManager_.sendMessage(u->getUserID(), u->getUserID(), receiverID, text);

        cout << "\nMessage sent.\n";
    }
    catch (const SkillBridgeException& e) {
        cout << "\nError: " << e.what() << "\n";
    }
}

void CLIApp::doViewInbox() {
    printHeader("Inbox");
    User* u = userManager_.getCurrentUser();
    if (!u) 
        return;
    try {
        DataList<DecodedMessage> inbox = messageManager_.getInbox(u->getUserID(), u->getUserID());

        if (inbox.size() == 0) { 
            cout << "Inbox is empty.\n"; 
            return; 
        }

        cout << "(newest first)\n\n";
        for (int i = 0; i < inbox.size(); ++i) {
            const DecodedMessage& m = inbox.get(i);
            cout << "  [#" << m.messageID << "] from user " << m.senderID << " at " << m.timestamp << (m.isRead ? "  (read)" : "  (unread)") << "\n";
            cout << "    " << m.text << "\n\n";
        }
        cout << "(total: " << inbox.size() << ")\n";
    }
    catch (const SkillBridgeException& e) {
        cout << "\nError: " << e.what() << "\n";
    }
}

void CLIApp::doViewConversation() {
    printHeader("Conversation");
    User* u = userManager_.getCurrentUser();
    if (!u) 
        return;
    try {
        int otherID = readInt("Other user's ID: ");
        if (otherID <= 0) {
            cout << "Invalid user ID.\n";
            return;
        }

        DataList<DecodedMessage> conv = messageManager_.getConversation(u->getUserID(), u->getUserID(), otherID);

        if (conv.size() == 0) {
            cout << "No messages yet between you and user " << otherID << ".\n";
            return;
        }

        cout << "(oldest first)\n\n";
        for (int i = 0; i < conv.size(); ++i) {
            const DecodedMessage& m = conv.get(i);
            const char* arrow = (m.senderID == u->getUserID()) ? "->" : "<-";
            cout << "  " << m.timestamp << "  " << arrow << " user " << (arrow[0] == '-' ? m.receiverID : m.senderID) << (m.isRead ? "  (read)" : "  (unread)") << "\n";
            cout << "    " << m.text << "\n\n";
        }
        cout << "(total: " << conv.size() << ")\n";
    }
    catch (const SkillBridgeException& e) {
        cout << "\nError: " << e.what() << "\n";
    }
}

void CLIApp::doMarkConversationRead() {
    printHeader("Mark Conversation Read");
    User* u = userManager_.getCurrentUser();
    if (!u) 
        return;
    try {
        int otherID = readInt("Other user's ID: ");
        if (otherID <= 0) { 
            cout << "Invalid user ID.\n"; 
            return; 
        }

        int n = messageManager_.markConversationRead(u->getUserID(), u->getUserID(), otherID);
        cout << "\nMarked " << n << " message(s) as read.\n";
    }
    catch (const SkillBridgeException& e) {
        cout << "\nError: " << e.what() << "\n";
    }
}

void CLIApp::showReviewMenu() {
    User* u = userManager_.getCurrentUser();
    if (!u) return;
    switch (u->getRole()) {
    case UserRole::CLIENT: showClientReviewMenu();
        break;
    case UserRole::FREELANCER: showFreelancerReviewMenu();
        break;
    case UserRole::ADMIN: showAdminReviewMenu();      
        break;
    default: cout << "Unknown role.\n"; break;
    }
}

void CLIApp::showClientReviewMenu() {
    while (userManager_.isLoggedIn()) {
        cout << "\n-- Reviews (Client) --\n";
        cout << "1. Submit review on a completed order\n";
        cout << "2. View reviews for a freelancer\n";
        cout << "3. Back\n";
        int choice = readMenuChoice();
        switch (choice) {
        case 1: doSubmitReview();             
            break;
        case 2: doViewReviewsForFreelancer(); 
            break;
        case 3: 
            return;
        default: cout << "Invalid choice. Try again.\n"; 
            break;
        }
    }
}

void CLIApp::showFreelancerReviewMenu() {
    while (userManager_.isLoggedIn()) {
        cout << "\n-- Reviews (Freelancer) --\n";
        cout << "1. View reviews about me\n";
        cout << "2. View reviews for a freelancer\n";
        cout << "3. My average rating\n";
        cout << "4. Back\n";
        int choice = readMenuChoice();
        switch (choice) {
        case 1: doViewReviewsAboutMe();       
            break;
        case 2: doViewReviewsForFreelancer(); 
            break;
        case 3: doMyAverageRating();          
            break;
        case 4: 
            return;
        default: cout << "Invalid choice. Try again.\n"; break;
        }
    }
}

void CLIApp::showAdminReviewMenu() {
    while (userManager_.isLoggedIn()) {
        cout << "\n-- Reviews (Admin) --\n";
        cout << "1. View reviews for a freelancer\n";
        cout << "2. Delete a review\n";
        cout << "3. Back\n";
        int choice = readMenuChoice();
        switch (choice) {
        case 1: doViewReviewsForFreelancer(); 
            break;
        case 2: doAdminDeleteReview();        
            break;
        case 3: 
            return;
        default: cout << "Invalid choice. Try again.\n"; 
            break;
        }
    }
}

static void printReviewSummary(const Review& r) {
    cout << "  [#" << r.getReviewID() << "] order #" << r.getOrderID()
        << " | reviewer=" << r.getReviewerID()
        << " | target=" << r.getTargetUserID()
        << " | " << r.getRating() << "/5"
        << " | sentiment=";
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%+.2f", r.getSentimentScore());
    cout << buf
        << "\n    \"" << r.getComment() << "\"\n";
}

void CLIApp::doSubmitReview() {
    printHeader("Submit Review");
    User* u = userManager_.getCurrentUser();
    if (!u)
        return;
    try {
        int orderID = readInt("Completed order ID: ");
        if (orderID <= 0) { cout << "Invalid order ID.\n"; return;
        }

        int rating = readInt("Rating (1-5): ");
        if (rating < 1 || rating > 5) {
            cout << "Rating must be between 1 and 5.\n";
            return;
        }
        string comment = readLine("Comment: ");
        Review saved = reviewManager_.submitReview(u->getUserID(),
            orderID, rating, comment);
        cout << "\nReview submitted.\n";
        printReviewSummary(saved);
    }
    catch (const SkillBridgeException& e) {
        cout << "\nError: " << e.what() << "\n";
    }
}

void CLIApp::doViewReviewsForFreelancer() {
    printHeader("Reviews for Freelancer");
    User* u = userManager_.getCurrentUser();
    if (!u)
        return;
    try {
        int targetID = readInt("Freelancer user ID: ");
        if (targetID <= 0) {
            cout << "Invalid user ID.\n";
            return;
        }

        DataList<Review> reviews = reviewManager_.getReviewsForFreelancer(u->getUserID(), targetID);

        double avg = reviewManager_.getAverageRating(targetID);

        if (reviews.size() == 0) {
            cout << "No reviews yet for user " << targetID << ".\n";
            return;
        }

        char abuf[32];
        std::snprintf(abuf, sizeof(abuf), "%.2f", avg);

        cout << "\nAverage rating: " << abuf << "/5 across " << reviews.size() << " review(s)\n\n";

        for (int i = 0; i < reviews.size(); ++i)
            printReviewSummary(reviews.get(i));
    }
    catch (const SkillBridgeException& e) {
        cout << "\nError: " << e.what() << "\n";
    }
}

void CLIApp::doViewReviewsAboutMe() {
    printHeader("Reviews About Me");
    User* u = userManager_.getCurrentUser();
    if (!u)
        return;
    try {
        DataList<Review> reviews =
            reviewManager_.getReviewsForFreelancer(u->getUserID(),
                u->getUserID());

        if (reviews.size() == 0) {
            cout << "No reviews about you yet.\n";
            return;
        }

        double avg = reviewManager_.getAverageRating(u->getUserID());
        char abuf[32];
        std::snprintf(abuf, sizeof(abuf), "%.2f", avg);

        cout << "\nAverage rating: " << abuf
            << "/5 across " << reviews.size() << " review(s)\n\n";

        for (int i = 0; i < reviews.size(); ++i)
            printReviewSummary(reviews.get(i));
    }
    catch (const SkillBridgeException& e) {
        cout << "\nError: " << e.what() << "\n";
    }
}

void CLIApp::doMyAverageRating() {
    printHeader("My Average Rating");
    User* u = userManager_.getCurrentUser();
    if (!u)
        return;
    double avg = reviewManager_.getAverageRating(u->getUserID());
    char abuf[32];
    std::snprintf(abuf, sizeof(abuf), "%.2f", avg);
    cout << "Average rating: " << abuf << "/5\n";
}

void CLIApp::doAdminDeleteReview() {
    printHeader("Delete Review (Admin)");
    User* u = userManager_.getCurrentUser();
    if (!u)
        return;
    try {
        int reviewID = readInt("Review ID to delete: ");
        if (reviewID <= 0) { 
            cout << "Invalid ID.\n"; 
            return; 
        }

        string confirm = readLine("Type 'DELETE' to confirm: ");
        if (confirm != "DELETE") { 
            cout << "Cancelled.\n"; 
            return; 
        }

        if (reviewManager_.adminDeleteReview(u->getUserID(), reviewID))
            cout << "\nReview deleted.\n";
        else
            cout << "\nDelete failed.\n";
    }
    catch (const SkillBridgeException& e) {
        cout << "\nError: " << e.what() << "\n";
    }
}

void CLIApp::showEndorsementMenu() {
    while (userManager_.isLoggedIn()) {
        cout << "\n-- Endorsements --\n";
        cout << "1. Endorse a user\n";
        cout << "2. View endorsements received by a user\n";
        cout << "3. Top freelancers (PageRank)\n";
        cout << "4. Trusted users near me (BFS)\n";
        cout << "5. Remove an endorsement I made\n";
        cout << "6. Back\n";
        int choice = readMenuChoice();
        switch (choice) {
        case 1: doEndorseUser();              
            break;
        case 2: doViewEndorsementsForUser();  
            break;
        case 3: doViewTopFreelancers();       
            break;
        case 4: doViewTrustedNear();          
            break;
        case 5: doRemoveMyEndorsement();      
            break;
        case 6: 
            return;
        default: cout << "Invalid choice. Try again.\n"; 
            break;
        }
    }
}

void CLIApp::doEndorseUser() {
    printHeader("Endorse a User");
    User* u = userManager_.getCurrentUser();
    if (!u) 
        return;
    try {
        int targetID = readInt("Target user ID: ");
        if (targetID <= 0) { 
            cout << "Invalid user ID.\n"; 
            return; 
        }

        string skill = readLine("Skill (e.g. 'C++', 'design'): ");
        if (skill.empty()) { 
            cout << "Skill cannot be empty.\n"; 
        return; 
        }

        string raw = readLine("Weight (0 < w <= 10, empty = 1.0): ");
        double weight = 1.0;
        if (!raw.empty()) {
            try { weight = stod(raw); }
            catch (...) { cout << "Invalid weight.\n"; return; }
        }

        Endorsement e = skillGraphManager_.endorseUser(u->getUserID(),
            targetID, skill, weight);

        cout << "\nEndorsement #" << e.getEndorsementID()
            << " recorded for user " << e.getToUserID()
            << " on skill '" << e.getSkill() << "'"
            << " with weight " << e.getWeight() << ".\n";
    }
    catch (const SkillBridgeException& e) {
        cout << "\nError: " << e.what() << "\n";
    }
}

void CLIApp::doViewEndorsementsForUser() {
    printHeader("Endorsements For User");
    User* u = userManager_.getCurrentUser();
    if (!u) 
        return;
    try {
        int targetID = readInt("Target user ID: ");
        if (targetID <= 0) { 
            cout << "Invalid user ID.\n"; 
            return; 
        }

        DataList<Endorsement> all =
            skillGraphManager_.getEndorsementsFor(u->getUserID(), targetID);

        if (all.size() == 0) {
            cout << "No endorsements yet for user " << targetID << ".\n";
            return;
        }

        cout << "\n";
        for (int i = 0; i < all.size(); ++i) {
            const Endorsement& e = all.get(i);
            cout << "  [#" << e.getEndorsementID() << "] from user "
                << e.getFromUserID() << " for '" << e.getSkill()
                << "' (weight " << e.getWeight() << ")"
                << " at " << e.getTimestamp() << "\n";
        }
        cout << "\n(total: " << all.size() << ")\n";
    }
    catch (const SkillBridgeException& e) {
        cout << "\nError: " << e.what() << "\n";
    }
}

void CLIApp::doViewTopFreelancers() {
    printHeader("Top Freelancers (PageRank)");
    User* u = userManager_.getCurrentUser();
    if (!u) 
        return;
    try {
        int n = readInt("How many to show? (default 10): ");
        if (n <= 0) 
            n = 10;
        DataList<RankedUser> top =
            skillGraphManager_.getTopFreelancers(u->getUserID(), n);
        if (top.size() == 0) {
            cout << "No ranking available; the endorsement graph is empty.\n";
            return;
        }
        cout << "\n";
        for (int i = 0; i < top.size(); ++i) {
            const RankedUser& ru = top.get(i);
            char sbuf[32];
            std::snprintf(sbuf, sizeof(sbuf), "%.6f", ru.score);
            cout << "  " << (i + 1) << ". user " << ru.userID << "  | score=" << sbuf << "\n";
        }
        cout << "\n(showing top " << top.size() << ")\n";
    }
    catch (const SkillBridgeException& e) {
        cout << "\nError: " << e.what() << "\n";
    }
}

void CLIApp::doViewTrustedNear() {
    printHeader("Trusted Users Near Me (BFS)");
    User* u = userManager_.getCurrentUser();
    if (!u) 
        return;
    try {
        int hops = readInt("Max hops (default 2): ");
        if (hops <= 0) 
            hops = 2;
        DataList<TrustedUser> near = skillGraphManager_.getTrustedNear(u->getUserID(), hops);
        if (near.size() == 0) {
            cout << "No trusted users found within " << hops << " hop(s) of you.\n";
            return;
        }
        cout << "\n";
        for (int i = 0; i < near.size(); ++i) {
            const TrustedUser& t = near.get(i);
            cout << "  user " << t.userID
                << "  | " << t.hopCount << " hop(s) away\n";
        }
        cout << "\n(total: " << near.size() << ")\n";
    }
    catch (const SkillBridgeException& e) {
        cout << "\nError: " << e.what() << "\n";
    }
}

void CLIApp::doRemoveMyEndorsement() {
    printHeader("Remove an Endorsement");
    User* u = userManager_.getCurrentUser();
    if (!u) 
        return;
    try {
        int eid = readInt("Endorsement ID to remove: ");
        if (eid <= 0) { 
            cout << "Invalid ID.\n"; 
            return; 
        }

        if (skillGraphManager_.removeEndorsement(u->getUserID(), eid))
            cout << "\nEndorsement removed.\n";
        else
            cout << "\nRemoval failed.\n";
    }
    catch (const SkillBridgeException& e) {
        cout << "\nError: " << e.what() << "\n";
    }
}

void CLIApp::showAdminPanelMenu() {
    while (userManager_.isLoggedIn()) {
        cout << "\n-- Admin Panel --\n";
        cout << "1. View all users\n";
        cout << "2. Delete a user\n";
        cout << "3. Back\n";
        int choice = readMenuChoice();
        switch (choice) {
        case 1: doAdminListAllUsers();
            break;
        case 2: doAdminDeleteUser();
            break;
        case 3:
            return;
        default: cout << "Invalid choice. Try again.\n";
            break;
        }
    }
}

void CLIApp::doAdminListAllUsers() {
    printHeader("All Users (Admin)");
    User* u = userManager_.getCurrentUser();
    if (!u)
        return;
    try {
        DataList<User*> all = userManager_.adminListAllUsers(u->getUserID());
        if (all.size() == 0) {
            cout << "No users in the system.\n";
            return;
        }
        for (int i = 0; i < all.size(); ++i) {
            User* x = all.get(i);
            cout << "  [#" << x->getUserID() << "] "
                << x->getName()
                << "  | " << x->getEmail()
                << "  | " << roleLabel(*x)
                << "  | " << formatRs(x->getBalance())
                << "  | joined " << x->getCreatedAt()
                << "\n";
        }
        cout << "\n(total: " << all.size() << ")\n";
        for (int i = 0; i < all.size(); ++i)
            delete all.get(i);
    }
    catch (const SkillBridgeException& e) {
        cout << "\nError: " << e.what() << "\n";
    }
}
void CLIApp::doAdminDeleteUser() {
    printHeader("Delete User (Admin)");
    User* u = userManager_.getCurrentUser();
    if (!u) 
        return;
    try {
        int targetID = readInt("Target user ID: ");
        if (targetID <= 0) { cout << "Invalid user ID.\n"; return; }
        cout << "Warning: this permanently deletes the user and cascades to their gigs, orders, messages, reviews, and endorsements.\n";
        string confirm = readLine("Type 'DELETE' to confirm: ");
        if (confirm != "DELETE") {
            cout << "Cancelled.\n"; 
            return; }
        if (userManager_.adminDeleteUser(u->getUserID(), targetID))
            cout << "\nUser deleted.\n";
        else
            cout << "\nDelete failed (user not found).\n";
    }
    catch (const SkillBridgeException& e) {
        cout << "\nError: " << e.what() << "\n";
    }
}