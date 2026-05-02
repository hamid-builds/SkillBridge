#ifndef SKILLBRIDGE_ORDER_STATE_MACHINE_H
#define SKILLBRIDGE_ORDER_STATE_MACHINE_H

#include "core/OrderStatus.h"
#include "utils/DataList.h"



class OrderStateMachine {
public:
    OrderStateMachine() = default;
    bool canTransition(OrderStatus from, OrderStatus to) const;
    DataList<OrderStatus> legalNextStates(OrderStatus from) const;
    bool isTerminal(OrderStatus s) const;
};

#endif