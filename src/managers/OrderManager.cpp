#include "managers/OrderManager.h"
#include "core/User.h"
#include "core/Gig.h"
#include "core/Exceptions.h"
#include "utils/MinHeap.h"

#include <ctime>
#include <cstdio>
#include <string>

using namespace std;

string OrderManager::currentTimestamp() const {
   
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
    return string(buf);
}

User* OrderManager::loadUser(int userID) const {
    User* u = userRepo_.findUserByID(userID);
    if (u == nullptr) {
        throw ValidationException(
            "User not found: userID=" + to_string(userID));
    }
    return u;
}

void OrderManager::requireRole(const User* u, UserRole expected) const {
    if (u->getRole() != expected) {
        throw UnauthorizedException(
            "Role mismatch: expected " + roleToString(expected) +
            " but caller is " + roleToString(u->getRole()));
    }
}

void OrderManager::requireBuyerSellerOrAdmin(const User* u, const Order& o) const {
    bool isBuyer = (u->getUserID() == o.getBuyerID());
    bool isSeller = (u->getUserID() == o.getSellerID());
    bool isAdmin = (u->getRole() == UserRole::ADMIN);
    if (!isBuyer && !isSeller && !isAdmin) {
        throw UnauthorizedException(
            "User " + to_string(u->getUserID()) +
            " is not buyer, seller, or admin for order " +
            to_string(o.getOrderID()));
    }
}

void OrderManager::requireAdmin(const User* u) const {
    if (u->getRole() != UserRole::ADMIN) {
        throw UnauthorizedException("Admin role required");
    }
}

void OrderManager::validateDeadline(const string& deadline) const {
    
    if (deadline.size() != 10) {
        throw ValidationException(
            "Deadline must be in YYYY-MM-DD format (got '" + deadline + "')");
    }
   
    for (int i = 0; i < 10; ++i) {
        char c = deadline[i];
        if (i == 4 || i == 7) {
            if (c != '-') {
                throw ValidationException(
                    "Deadline format error at position " + to_string(i) +
                    ": expected '-' got '" + string(1, c) + "'");
            }
        }
        else {
            if (c < '0' || c > '9') {
                throw ValidationException(
                    "Deadline format error at position " + to_string(i) +
                    ": expected digit got '" + string(1, c) + "'");
            }
        }
    }
    
    int month = (deadline[5] - '0') * 10 + (deadline[6] - '0');
    int day = (deadline[8] - '0') * 10 + (deadline[9] - '0');
    if (month < 1 || month > 12) {
        throw ValidationException("Deadline month out of range: " + to_string(month));
    }
    if (day < 1 || day > 31) {
        throw ValidationException("Deadline day out of range: " + to_string(day));
    }
    
    string today = currentTimestamp().substr(0, 10);  
    if (deadline < today) {
        throw ValidationException(
            "Deadline must be today or later: got " + deadline +
            ", today is " + today);
    }
}



OrderManager::OrderManager(IOrderRepository& orderRepo,
    IUserRepository& userRepo,
    IGigRepository& gigRepo,
    OrderStateMachine& stateMachine)
    : orderRepo_(orderRepo),
    userRepo_(userRepo),
    gigRepo_(gigRepo),
    stateMachine_(stateMachine)
{
}

Order OrderManager::placeOrder(int currentUserID,
    int gigID,
    const string& deadline) {
    
    validateDeadline(deadline);
    User* buyer = loadUser(currentUserID);

    
    try {
        
        requireRole(buyer, UserRole::CLIENT);

        
        Gig gig = gigRepo_.findGigByID(gigID);
        if (!gig.getIsActive()) 
        {
            throw ValidationException(
                "Cannot order an inactive gig (gigID=" + to_string(gigID) + ")");
        }
        if (gig.getOwnerID() == currentUserID) 
        {
            throw UnauthorizedException(
                "Cannot place an order on your own gig (gigID=" +
                to_string(gigID) + ")");
        }

        double price = gig.getPrice();

        buyer->withdraw(price);

        if (!userRepo_.updateUser(buyer)) 
        {
            buyer->deposit(price);  
            throw DatabaseException(
                "Failed to persist buyer balance for userID=" +
                to_string(currentUserID));
        }

        const string placedAt = currentTimestamp();
        Order order(gigID,
            currentUserID,
            gig.getOwnerID(),
            price,
            placedAt,
            deadline);

        try {
            orderRepo_.saveOrder(order);
        }
        catch (...) {
           
            buyer->deposit(price);
            userRepo_.updateUser(buyer);
            throw;
        }

        delete buyer;
        return order;
    }
    catch (...) {
        delete buyer;
        throw;
    }
}

void OrderManager::updateStatus(int currentUserID,
    int orderID,
    OrderStatus newStatus) {
  
    Order order = orderRepo_.findOrderByID(orderID);

    User* caller = loadUser(currentUserID);
    try {
        requireBuyerSellerOrAdmin(caller, order);

        
        if (!stateMachine_.canTransition(order.getStatus(), newStatus))
        {
            throw ValidationException(
                "Illegal transition: " +
                orderStatusToString(order.getStatus()) +
                " -> " + orderStatusToString(newStatus));
        }

        string completedAtTimestamp = "";

        if (newStatus == OrderStatus::COMPLETED) 
        {
            
            User* seller = loadUser(order.getSellerID());
            try {
                seller->deposit(order.getAmount());
                if (!userRepo_.updateUser(seller)) {
                    
                    seller->withdraw(order.getAmount());
                    throw DatabaseException(
                        "Failed to persist seller balance");
                }
                delete seller;
            }
            catch (...) {
                delete seller;
                throw;
            }
            completedAtTimestamp = currentTimestamp();
        }
        else if (newStatus == OrderStatus::CANCELLED)
        {

            User* buyer = loadUser(order.getBuyerID());
            try {
                buyer->deposit(order.getAmount());
                if (!userRepo_.updateUser(buyer)) {
                    buyer->withdraw(order.getAmount());
                    throw DatabaseException(
                        "Failed to persist buyer refund");
                }
                delete buyer;
            }
            catch (...) {
                delete buyer;
                throw;
            }
        }

        orderRepo_.updateOrderStatus(orderID, newStatus, completedAtTimestamp);

        delete caller;
    }
    catch (...) 
    {
        delete caller;
        throw;
    }
}

void OrderManager::cancelOrder(int currentUserID, int orderID) 
{
    updateStatus(currentUserID, orderID, OrderStatus::CANCELLED);
}



Order OrderManager::findOrderByID(int currentUserID, int orderID) const 
{
    Order o = orderRepo_.findOrderByID(orderID);
    User* caller = loadUser(currentUserID);
    try {
        requireBuyerSellerOrAdmin(caller, o);
        delete caller;
        return o;
    }
    catch (...) {
        delete caller;
        throw;
    }
}
Order OrderManager::findOrderByIDForCommand(int orderID) const {
    return orderRepo_.findOrderByID(orderID);
}

DataList<Order> OrderManager::findByBuyer(int currentUserID) const 
{
    
    User* caller = loadUser(currentUserID);
    delete caller;
    return orderRepo_.findOrdersByBuyer(currentUserID);
}

DataList<Order> OrderManager::findBySeller(int currentUserID) const 
{
    User* caller = loadUser(currentUserID);
    delete caller;
    return orderRepo_.findOrdersBySeller(currentUserID);
}

DataList<Order> OrderManager::findAll(int currentUserID) const 
{
    User* caller = loadUser(currentUserID);
    try {
        requireAdmin(caller);
        delete caller;
        return orderRepo_.findAllOrders();
    }
    catch (...) {
        delete caller;
        throw;
    }
}


void OrderManager::hardDeleteOrderForUndo(int orderID) {
    Order order = orderRepo_.findOrderByID(orderID); 

    int buyerID = order.getBuyerID();
    double amount = order.getAmount();

    User* buyer = userRepo_.findUserByID(buyerID);
    if (!buyer) {
        throw OrderNotFoundException(
            "Buyer for order " + std::to_string(orderID) + " not found during undo");
    }

    try {
        buyer->deposit(amount);
        if (!userRepo_.updateUser(buyer)) {
            throw DatabaseException(
                "Failed to persist buyer refund during placeOrder undo");
        }
        orderRepo_.deleteOrder(orderID);  
    }
    catch (...) {
        delete buyer;
        throw;
    }
    delete buyer;
}

void OrderManager::undoCancellationForUndo(int orderID,
    OrderStatus statusBeforeCancel) {
    Order order = orderRepo_.findOrderByID(orderID);

    if (order.getStatus() != OrderStatus::CANCELLED) {
        throw ValidationException(
            "Cannot undo cancellation: order " + std::to_string(orderID) +
            " is not in CANCELLED state");
    }

    int buyerID = order.getBuyerID();
    double amount = order.getAmount();

    User* buyer = userRepo_.findUserByID(buyerID);
    if (!buyer) {
        throw OrderNotFoundException(
            "Buyer for order " + std::to_string(orderID) + " not found during undo");
    }

    if (buyer->getBalance() < amount) {
        double available = buyer->getBalance();
        delete buyer;
        throw InsufficientFundsException(amount, available);
    }

    try {
        buyer->withdraw(amount);
        if (!userRepo_.updateUser(buyer)) {
            throw DatabaseException(
                "Failed to persist buyer re-deduction during cancellation undo");
        }
        order.setStatus(statusBeforeCancel);
        orderRepo_.updateOrder(order);
    }
    catch (...) {
        delete buyer;
        throw;
    }
    delete buyer;
}

void OrderManager::undoStatusChangeForUndo(int orderID, OrderStatus oldStatus, OrderStatus newStatusThatWasApplied)
{
    Order order = orderRepo_.findOrderByID(orderID);

    if (order.getStatus() != newStatusThatWasApplied) {
        throw ValidationException(
            "Cannot undo status change: order " + std::to_string(orderID) +
            " is not in the expected post-execute state");
    }

    bool needsSellerDebit = (newStatusThatWasApplied == OrderStatus::COMPLETED);
    User* seller = nullptr;
    double amount = order.getAmount();

    if (needsSellerDebit) {
        seller = userRepo_.findUserByID(order.getSellerID());
        if (!seller) {
            throw OrderNotFoundException(
                "Seller for order " + std::to_string(orderID) +
                " not found during undo");
        }
        if (seller->getBalance() < amount) {
            double available = seller->getBalance();
            delete seller;
            throw InsufficientFundsException(amount, available);
        }
    }

    try {
        if (needsSellerDebit) {
            seller->withdraw(amount);
            if (!userRepo_.updateUser(seller)) {
                throw DatabaseException(
                    "Failed to persist seller debit during status undo");
            }
        }
        order.setStatus(oldStatus);
        orderRepo_.updateOrder(order);
    }
    catch (...) {
        delete seller;  
        throw;
    }
    delete seller;  
}

DataList<Order> OrderManager::findUrgentOrders(int currentUserID,
    int topK) const {
    if (topK <= 0) {
        throw ValidationException( "topK must be positive (got " + to_string(topK) + ")");
    }

    User* caller = loadUser(currentUserID);
    DataList<Order> candidates;
    try {
        bool isAdmin = (caller->getRole() == UserRole::ADMIN);

        if (isAdmin)
        {
            candidates = orderRepo_.findAllOrders();
        }
        else {
            
            DataList<Order> asBuyer = orderRepo_.findOrdersByBuyer(currentUserID);
            DataList<Order> asSeller = orderRepo_.findOrdersBySeller(currentUserID);
            for (int i = 0; i < asBuyer.size(); ++i)  candidates.add(asBuyer.get(i));
            for (int i = 0; i < asSeller.size(); ++i) candidates.add(asSeller.get(i));
        }

        delete caller;
    }
    catch (...) {
        delete caller;
        throw;
    }
   
    struct DeadlineGreater {
        bool operator()(const Order& a, const Order& b) const {
            return b < a;  
        }
    };

    MinHeap<Order, DeadlineGreater> heap{ DeadlineGreater() };

    for (int i = 0; i < candidates.size(); ++i) {
        const Order& o = candidates.get(i);

        if (o.getStatus() == OrderStatus::COMPLETED ||
            o.getStatus() == OrderStatus::CANCELLED) {
            continue;
        }

        heap.push(o);
        if (heap.size() > topK) {
            heap.pop();  
        }
    }

    DataList<Order> reversed;
    while (!heap.isEmpty()) {
        reversed.add(heap.pop());
    }

    DataList<Order> result;
    for (int i = reversed.size() - 1; i >= 0; --i) {
        result.add(reversed.get(i));
    }
    return result;
}