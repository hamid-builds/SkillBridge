#ifndef SKILLBRIDGE_CLIAPP_H
#define SKILLBRIDGE_CLIAPP_H

#include <string>
#include "../core/GigCategory.h"

class UserManager;
class GigManager;
class Gig;



class CLIApp {
private:
    UserManager& userManager_;
    GigManager& gigManager_;
    bool         running_;

    
    void showLoggedOutMenu();
    void showLoggedInMenu();

    
    void doRegister();
    void doLogin();

   
    void showProfileMenu();
    void doViewProfile();
    void doUpdateName();
    void doChangePassword();
    void doDeleteAccount();
    void doLogout();

   
    void showGigMenu();
    void showFreelancerGigMenu();
    void showClientGigMenu();
    void showAdminGigMenu();

   
    void doBrowseGigs();
    void doSearchGigs();

   
    void doMyGigs();
    void doCreateGig();
    void doEditMyGig(int gigID);
    void doDeactivateMyGig(int gigID);

    
    void doAutocompleteDemo();

    
    void doViewAllGigs();
    void doHardDeleteGig();

   
    int          readMenuChoice();
    int          readInt(const std::string& prompt);
    double       readDouble(const std::string& prompt);
    std::string  readLine(const std::string& prompt);
    void         printHeader(const std::string& title);

    
    void printGigSummary(const Gig& gig);

    
    void printGigDetail(const Gig& gig);

    
    bool readCategory(GigCategory& out);

public:
    
    CLIApp(UserManager& userManager, GigManager& gigManager);

    
    void run();
};

#endif