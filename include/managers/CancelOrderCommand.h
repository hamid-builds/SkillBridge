#ifndef SKILLBRIDGE_CANCELORDERCOMMAND_H
#define SKILLBRIDGE_CANCELORDERCOMMAND_H

#include "ICommand.h"
#include "../core/OrderStatus.h"


class OrderManager;

class CancelOrderCommand : public ICommand
{
private:
    OrderManager& mgr_;
    int currentUserID_;
    int orderID_;

    OrderStatus statusBeforeCancel_;
    bool hasExecuted_;

public:
    CancelOrderCommand(OrderManager& mgr, int currentUserID, int orderID);

    void execute() override;
    void undo() override;
    std::string description() const override;
};

#endif