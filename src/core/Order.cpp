#include "core/Order.h"
#include "core/Exceptions.h"
#include <ostream>
using namespace std;
// Helper used by validating constructors only. Setters do their own
// per-field validation; cross-field check (buyer != seller) lives here.

static void validateOrderFields(int gigID, int buyerID, int sellerID, double amount, const string& placedAt, const string& deadline) 
{
    if (gigID <= 0) 
        throw ValidationException("Order: gigID must be positive");
    if (buyerID <= 0) 
        throw ValidationException("Order: buyerID must be positive");
    if (sellerID <= 0)
        throw ValidationException("Order: sellerID must be positive");
    if (buyerID == sellerID) {
    
        throw ValidationException("Order: buyer and seller must differ");
    }
    if (amount <= 0.0) 
        throw ValidationException("Order: amount must be positive");
    if (placedAt.empty())
        throw ValidationException("Order: placedAt cannot be empty");
    if (deadline.empty()) 
        throw ValidationException("Order: deadline cannot be empty");
}


Order::Order() : orderID_(0), gigID_(0),buyerID_(0),sellerID_(0),amount_(0.0),status_(OrderStatus::PENDING),placedAt_(""),completedAt_(""),
deadline_("")
{
    
}



Order::Order(int orderID,  int gigID, int buyerID, int sellerID, double amount, OrderStatus status, const string& placedAt, const string& completedAt,
    const string& deadline): orderID_(orderID),gigID_(gigID),buyerID_(buyerID),sellerID_(sellerID),amount_(amount),status_(status),placedAt_(placedAt),
    completedAt_(completedAt),deadline_(deadline)
{
    if (orderID < 0)
        throw ValidationException("Order: orderID cannot be negative");
    validateOrderFields(gigID, buyerID, sellerID, amount, placedAt, deadline);
    
}


Order::Order(int gigID,int buyerID,int sellerID,double amount,const string& placedAt,const string& deadline): orderID_(0),gigID_(gigID),buyerID_(buyerID),
    sellerID_(sellerID),amount_(amount),status_(OrderStatus::PENDING),placedAt_(placedAt),completedAt_(""),deadline_(deadline)
{
    validateOrderFields(gigID, buyerID, sellerID, amount, placedAt, deadline);
}

void Order::setOrderID(int id)
{
    if (id <= 0) 
    {
        throw ValidationException("Order::setOrderID: id must be positive");
    }
    if (orderID_ != 0 && orderID_ != id) 
    {
        throw ValidationException("Order::setOrderID: orderID is already assigned");
    }
    orderID_ = id;
}
void Order::setGigID(int gigID)
{
    if (gigID <= 0) 
        throw ValidationException("Order: gigID must be positive");
    gigID_ = gigID;
}

void Order::setBuyerID(int buyerID) {
    if (buyerID <= 0)
        throw ValidationException("Order: buyerID must be positive");
    buyerID_ = buyerID;
}

void Order::setSellerID(int sellerID) {
    if (sellerID <= 0)
        throw ValidationException("Order: sellerID must be positive");
    sellerID_ = sellerID;
}

void Order::setAmount(double amount) {
    if (amount <= 0.0) 
        throw ValidationException("Order: amount must be positive");
    amount_ = amount;
}

void Order::setPlacedAt(const string& t) {
    if (t.empty()) 
        throw ValidationException("Order: placedAt cannot be empty");
    placedAt_ = t;
}

void Order::setDeadline(const string& d) {
    if (d.empty()) 
        throw ValidationException("Order: deadline cannot be empty");
    deadline_ = d;
}



ostream& operator<<(ostream& os, const Order& o)
{
    os << "Order #" << o.orderID_<< " [gig=" << o.gigID_<< ", buyer=" << o.buyerID_<< ", seller=" << o.sellerID_<< ", amount=Rs." << o.amount_
        << ", status=" << orderStatusToString(o.status_)<< ", deadline=" << o.deadline_<< "]";
    return os;
}