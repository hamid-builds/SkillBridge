#ifndef SKILLBRIDGE_SQLITEORDERREPOSITORY_H
#define SKILLBRIDGE_SQLITEORDERREPOSITORY_H

#include "IOrderRepository.h"

struct sqlite3_stmt;

class SQLiteOrderRepository : public IOrderRepository {
public:
    SQLiteOrderRepository() = default;
    ~SQLiteOrderRepository() override = default;

    void saveOrder(Order& order) override;
    void updateOrder(const Order& order) override;
    void updateOrderStatus(int orderID,
        OrderStatus newStatus,
        const std::string& completedAt) override;
    bool deleteOrder(int orderID) override;

    Order        findOrderByID(int orderID) const override;
    DataList<Order>   findOrdersByBuyer(int buyerID) const override;
    DataList<Order>  findOrdersBySeller(int sellerID) const override;
    DataList<Order>   findOrdersByGig(int gigID) const override;
    DataList<Order>   findAllOrders() const override;

private:
    
    Order buildOrderFromRow(sqlite3_stmt* stmt) const;
    void throwPrepareError(const std::string& sql) const;
};

#endif