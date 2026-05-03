#ifndef SKILLBRIDGE_ORDER_H
#define SKILLBRIDGE_ORDER_H

#include <string>
#include <iosfwd>
#include "core/OrderStatus.h"


class Order {
private:
    int orderID_;
    int gigID_;
    int buyerID_;
    int sellerID_;
    double  amount_;
    OrderStatus status_;
    std::string placedAt_;
    std::string completedAt_;
    std::string deadline_;

public:
    Order();
    Order(int orderID,int gigID, int buyerID, int sellerID, double amount, OrderStatus status, const std::string& placedAt, const std::string& completedAt, const std::string& deadline);
    Order(int gigID, int buyerID,int sellerID, double amount, const std::string& placedAt,const std::string& deadline);

    //Getters
    int getOrderID() const
    { 
        return orderID_; 
    }
    int getGigID() const
    { 
        return gigID_; 
    }
    int  getBuyerID() const
    { 
        return buyerID_; 
    }
    int  getSellerID() const 
    { 
        return sellerID_;
    }
    double getAmount() const 
    {
        return amount_;
    }
    OrderStatus  getStatus() const 
    { 
        return status_;
    }
    const std::string& getPlacedAt() const 
    {
        return placedAt_; 
    }
    const std::string& getCompletedAt() const 
    {
        return completedAt_;
    }
    const std::string& getDeadline()  const 
    {
        return deadline_; 
    }
    //Setters

    void setOrderID(int id);

    void setGigID(int gigID);
    void setBuyerID(int buyerID);
    void setSellerID(int sellerID);
    void setAmount(double amount);

    void setStatus(OrderStatus s) 
    { 
        status_ = s; 
    }    
    void setPlacedAt(const std::string& t);
    void setCompletedAt(const std::string& t)
    {
        completedAt_ = t; 
    }
    void setDeadline(const std::string& d);
    

   //Operator
    bool operator==(const Order& other) const
    {
        return orderID_ == other.orderID_; 
    }
    bool operator!=(const Order& other) const 
    {
        return !(*this == other); 
    }

    bool operator<(const Order& other) const
    {
        return deadline_ < other.deadline_; 
    }
    friend std::ostream& operator<<(std::ostream& os, const Order& o);
};

#endif