#ifndef SKILLBRIDGE_ORDERMANAGER_H
#define SKILLBRIDGE_ORDERMANAGER_H

#include "IOrderRepository.h"
#include "IUserRepository.h"
#include "IGigRepository.h"
#include "OrderStateMachine.h"
#include "../core/Order.h"
#include "../core/UserRole.h"
#include "../utils/DataList.h"

class OrderManager {
private:
    IOrderRepository& orderRepo_;
    IUserRepository& userRepo_;
    IGigRepository& gigRepo_;
    OrderStateMachine& stateMachine_;

    std::string currentTimestamp() const;
   void requireRole(const User* u, UserRole expected) const;
    void requireBuyerSellerOrAdmin(const User* u, const Order& o) const;
    void requireAdmin(const User* u) const;
    User* loadUser(int userID) const;
    void validateDeadline(const std::string& deadline) const;

public:
    OrderManager(IOrderRepository& orderRepo,

    IUserRepository& userRepo,

    IGigRepository& gigRepo,

    OrderStateMachine& stateMachine);

    OrderManager(const OrderManager&) = delete;

    OrderManager& operator=(const OrderManager&) = delete;

    Order placeOrder(int currentUserID, int gigID, const std::string& deadline);

    void updateStatus(int currentUserID, int orderID, OrderStatus newStatus);

    void hardDeleteOrderForUndo(int orderID);

    void undoCancellationForUndo(int orderID, OrderStatus statusBeforeCancel);

   
    void undoStatusChangeForUndo(int orderID,
        OrderStatus oldStatus,
        OrderStatus newStatusThatWasApplied);

    void cancelOrder(int currentUserID, int orderID);

    Order findOrderByID(int currentUserID, int orderID) const;

    Order findOrderByIDForCommand(int orderID) const;

    DataList<Order> findByBuyer(int currentUserID) const;

    DataList<Order> findBySeller(int currentUserID) const;

    DataList<Order> findAll(int currentUserID) const;

    DataList<Order> findUrgentOrders(int currentUserID, int topK) const;
};

#endif