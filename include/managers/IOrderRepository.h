#ifndef SKILLBRIDGE_IORDERREPOSITORY_H
#define SKILLBRIDGE_IORDERREPOSITORY_H

#include "../core/Order.h"
#include "../utils/DataList.h"



class IOrderRepository {
public:
    virtual ~IOrderRepository() = default;

   
    virtual void saveOrder(Order& order) = 0;
    virtual void updateOrder(const Order& order) = 0;
    virtual void updateOrderStatus(int orderID,
        OrderStatus newStatus,
        const std::string& completedAt) = 0;

    virtual bool deleteOrder(int orderID) = 0;
     virtual Order findOrderByID(int orderID) const = 0;
    virtual DataList<Order> findOrdersByBuyer(int buyerID) const = 0;
    virtual DataList<Order> findOrdersBySeller(int sellerID) const = 0;
    virtual DataList<Order> findOrdersByGig(int gigID) const = 0;
    virtual DataList<Order> findAllOrders() const = 0;
};

#endif