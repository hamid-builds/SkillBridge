#include "managers/UpdateStatusCommand.h"
#include "managers/OrderManager.h"
#include "core/Order.h"

using namespace std;

UpdateStatusCommand::UpdateStatusCommand(OrderManager& mgr,int currentUserID,int orderID,OrderStatus newStatus)
    : mgr_(mgr), currentUserID_(currentUserID),orderID_(orderID),newStatus_(newStatus),oldStatus_(OrderStatus::PENDING),hasExecuted_(false) {
}

void UpdateStatusCommand::execute() 
{
    Order pre = mgr_.findOrderByIDForCommand(orderID_);
    oldStatus_ = pre.getStatus();

    mgr_.updateStatus(currentUserID_, orderID_, newStatus_);
    hasExecuted_ = true;
}

void UpdateStatusCommand::undo()
{
    if (!hasExecuted_) return;
    mgr_.undoStatusChangeForUndo(orderID_, oldStatus_, newStatus_);
    hasExecuted_ = false;
}

string UpdateStatusCommand::description() const
{
    return "Update order #" + to_string(orderID_) +
        " status to " + orderStatusToString(newStatus_);
}