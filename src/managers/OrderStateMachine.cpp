#include "managers/OrderStateMachine.h"

bool OrderStateMachine::canTransition(OrderStatus from, OrderStatus to) const 
{
    
    switch (from) {
    case OrderStatus::PENDING:
        return to == OrderStatus::IN_PROGRESS || to == OrderStatus::CANCELLED;

    case OrderStatus::IN_PROGRESS:
        return to == OrderStatus::COMPLETED || to == OrderStatus::CANCELLED;

    case OrderStatus::COMPLETED:
    case OrderStatus::CANCELLED:
         
        return false;
    }

    return false;
}

DataList<OrderStatus> OrderStateMachine::legalNextStates(OrderStatus from) const
{
    DataList<OrderStatus> result;

    switch (from)
    {
    case OrderStatus::PENDING:
        result.add(OrderStatus::IN_PROGRESS);
        result.add(OrderStatus::CANCELLED);
        break;

    case OrderStatus::IN_PROGRESS:
        result.add(OrderStatus::COMPLETED);
        result.add(OrderStatus::CANCELLED);
        break;

    case OrderStatus::COMPLETED:
    case OrderStatus::CANCELLED:
        
        break;
    }

    return result;
}

bool OrderStateMachine::isTerminal(OrderStatus s) const 
{
    return s == OrderStatus::COMPLETED || s == OrderStatus::CANCELLED;
}