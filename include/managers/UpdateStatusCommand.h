#ifndef SKILLBRIDGE_UPDATESTATUSCOMMAND_H
#define SKILLBRIDGE_UPDATESTATUSCOMMAND_H

#include "ICommand.h"
#include "../core/OrderStatus.h"

class OrderManager;

class UpdateStatusCommand : public ICommand {
private:
    OrderManager& mgr_;
    int currentUserID_;
    int orderID_;
    OrderStatus newStatus_;

    OrderStatus oldStatus_;
    bool hasExecuted_;

public:
    UpdateStatusCommand(OrderManager& mgr, int currentUserID, int orderID, OrderStatus newStatus);

    void execute() override;
    void undo() override;
    std::string description() const override;
};

#endif