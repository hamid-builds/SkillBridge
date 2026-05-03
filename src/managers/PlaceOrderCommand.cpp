#include "managers/PlaceOrderCommand.h"
#include "managers/OrderManager.h"
#include "core/Order.h"

using namespace std;

PlaceOrderCommand::PlaceOrderCommand(OrderManager& mgr, int currentUserID, int gigID, const string& deadline)
    : mgr_(mgr),currentUserID_(currentUserID), gigID_(gigID), deadline_(deadline), placedOrderID_(0), hasExecuted_(false)
{
}

void PlaceOrderCommand::execute() 
{
   
    Order placed = mgr_.placeOrder(currentUserID_, gigID_, deadline_);
    placedOrderID_ = placed.getOrderID();
    hasExecuted_ = true;
}

void PlaceOrderCommand::undo() 
{
    if (!hasExecuted_) {
       
        return;
    }
    mgr_.hardDeleteOrderForUndo(placedOrderID_);
    hasExecuted_ = false;  
}

string PlaceOrderCommand::description() const
{
    if (!hasExecuted_) {
        return "Place order on gig " + to_string(gigID_);
    }
    return "Place order #" + to_string(placedOrderID_) +
        " on gig " + to_string(gigID_);
}