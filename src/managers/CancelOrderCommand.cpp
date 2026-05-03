#include "managers/CancelOrderCommand.h"
#include "managers/OrderManager.h"
#include "core/Order.h"

using namespace std;

CancelOrderCommand::CancelOrderCommand(OrderManager& mgr,int currentUserID, int orderID)
    : mgr_(mgr), currentUserID_(currentUserID), orderID_(orderID), statusBeforeCancel_(OrderStatus::PENDING), hasExecuted_(false) 
{
}

void CancelOrderCommand::execute() 
{
   
    Order pre = mgr_.findOrderByIDForCommand(orderID_);
    statusBeforeCancel_ = pre.getStatus();

    mgr_.cancelOrder(currentUserID_, orderID_);
    hasExecuted_ = true;
}

void CancelOrderCommand::undo()
{
    if (!hasExecuted_) return;
    mgr_.undoCancellationForUndo(orderID_, statusBeforeCancel_);
    hasExecuted_ = false;
}

string CancelOrderCommand::description() const
{
    return "Cancel order #" + to_string(orderID_);
}