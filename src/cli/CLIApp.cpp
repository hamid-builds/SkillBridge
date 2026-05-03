#include "cli/CLIApp.h"
#include "managers/UserManager.h"
#include "managers/GigManager.h"
#include "managers/OrderManager.h"
#include "managers/CommandHistory.h"
#include "managers/PlaceOrderCommand.h"
#include "managers/CancelOrderCommand.h"
#include "managers/UpdateStatusCommand.h"
#include "core/User.h"
#include "core/UserRole.h"
#include "core/Gig.h"
#include "core/GigCategory.h"
#include "core/Order.h"
#include "core/OrderStatus.h"
#include "core/Exceptions.h"
#include "utils/DataList.h"

#include <iostream>
#include <iomanip>
#include <limits>
#include <string>
#include <stdexcept>

using namespace std;

CLIApp::CLIApp(UserManager& userManager,
    GigManager& gigManager,
    OrderManager& orderManager,
    CommandHistory& history)
    : userManager_(userManager),
    gigManager_(gigManager),
    orderManager_(orderManager),
    history_(history),
    running_(true) {
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

void CLIApp::printGigSummary(const Gig& gig) {
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

    cout << "1. My Profile\n";
    cout << "2. Gigs\n";
    cout << "3. Orders\n";
    cout << "4. Undo";
    if (!undoDesc.empty()) cout << "  (" << undoDesc << ")";
    cout << "\n";
    cout << "5. Redo";
    if (!redoDesc.empty()) cout << "  (" << redoDesc << ")";
    cout << "\n";
    cout << "6. Logout\n";
    cout << "7. Exit\n";

    int choice = readMenuChoice();
    switch (choice) {
    case 1: showProfileMenu(); 
        break;
    case 2: showGigMenu();    
        break;
    case 3: showOrderMenu();  
        break;
    case 4: doUndo();        
        break;
    case 5: doRedo();         
        break;
    case 6: doLogout();       
        break;
    case 7: running_ = false;  break;
    default: cout << "Invalid choice. Try again.\n"; break;
    }
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

    string roleStr;
    switch (u->getRole()) {
    case UserRole::CLIENT:     roleStr = "Client";   
        break;
    case UserRole::FREELANCER: roleStr = "Freelancer"; 
        break;
    case UserRole::ADMIN:      roleStr = "Admin";      
        break;
    default:                   roleStr = "Unknown";   
        break;
    }

    cout << "User ID:   " << u->getUserID() << "\n";
    cout << "Name:      " << u->getName() << "\n";
    cout << "Email:     " << u->getEmail() << "\n";
    cout << "Role:      " << roleStr << "\n";
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
    case UserRole::ADMIN:      showAdminOrderMenu();     
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