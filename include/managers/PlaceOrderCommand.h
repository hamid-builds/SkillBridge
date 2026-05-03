#ifndef SKILLBRIDGE_PLACEORDERCOMMAND_H
#define SKILLBRIDGE_PLACEORDERCOMMAND_H

#include "ICommand.h"
#include <string>

class OrderManager;  
class PlaceOrderCommand : public ICommand {
private:
    OrderManager& mgr_;
    int currentUserID_;
    int gigID_;
    std::string deadline_;

    int placedOrderID_;
    bool hasExecuted_;

public:
    PlaceOrderCommand(OrderManager& mgr,int currentUserID,int gigID,const std::string& deadline);

    void execute() override;
    void undo() override;
    std::string description() const override;
};

#endif