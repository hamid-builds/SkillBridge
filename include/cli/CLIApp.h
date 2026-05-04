#ifndef SKILLBRIDGE_CLIAPP_H
#define SKILLBRIDGE_CLIAPP_H

#include <string>
#include "../core/GigCategory.h"
#include "../core/OrderStatus.h"


class UserManager;
class GigManager;
class OrderManager;
class CommandHistory;
class MessageManager;
class ReviewManager;
class SkillGraphManager;
class Gig;
class Order;
class User;


class CLIApp {
private:
    UserManager& userManager_;
    GigManager& gigManager_;
    OrderManager& orderManager_;
    CommandHistory& history_;
    MessageManager& messageManager_;
    ReviewManager& reviewManager_;
    SkillGraphManager& skillGraphManager_;
    bool running_;

    void showLoggedOutMenu();
    void showLoggedInMenu();
    
    void doRegister();
    void doLogin();
   
    void showProfileMenu();
    void doViewProfile();
    void doUpdateName();
    void doChangePassword();
    void doDeleteAccount();
    void doDeposit();
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

    void showOrderMenu();
    void showClientOrderMenu();
    void showFreelancerOrderMenu();
    void showAdminOrderMenu();

    void doPlaceOrder();              
    void doViewMyOrdersAsBuyer();     
    void doViewIncomingOrders();      
    void doViewAllOrders();           
    void doCancelOrder();             
    void doUpdateOrderStatus();  
    void doViewUrgentOrders();        

    void doUndo();
    void doRedo();

    void showMessageMenu();
    void doSendMessage();
    void doViewInbox();
    void doViewConversation();
    void doMarkConversationRead();

    void showReviewMenu();
    void showClientReviewMenu();
    void showFreelancerReviewMenu();
    void showAdminReviewMenu();
    void doSubmitReview();
    void doViewReviewsForFreelancer();
    void doViewReviewsAboutMe();
    void doMyAverageRating();
    void doAdminDeleteReview();

    void showEndorsementMenu();
    void doEndorseUser();
    void doViewEndorsementsForUser();
    void doViewTopFreelancers();
    void doViewTrustedNear();
    void doRemoveMyEndorsement();

    void showAdminPanelMenu();
    void doAdminListAllUsers();
    void doAdminDeleteUser();
   
    int readMenuChoice();
    int readInt(const std::string& prompt);
    double readDouble(const std::string& prompt);
    std::string readLine(const std::string& prompt);
    void printHeader(const std::string& title);
    
    void printGigSummary(const Gig& gig);
    void printGigDetail(const Gig& gig);
    bool readCategory(GigCategory& out);

    void printOrderSummary(const Order& o);
    void printOrderDetail(const Order& o);

    bool readTargetStatus(OrderStatus& out);
    std::string roleLabel(const User& u) const;

public:
    
    CLIApp(UserManager& userManager, GigManager& gigManager, OrderManager& orderManager, CommandHistory& history, MessageManager& messageManager, ReviewManager& reviewManager, SkillGraphManager& skillGraphManager);
   
    void run();
};

#endif