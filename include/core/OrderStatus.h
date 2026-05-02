#ifndef SKILLBRIDGE_ORDER_STATUS_H
#define SKILLBRIDGE_ORDER_STATUS_H

#include <string>



enum class OrderStatus {
    PENDING,
    IN_PROGRESS,
    COMPLETED,
    CANCELLED
};

std::string orderStatusToString(OrderStatus status);
OrderStatus orderStatusFromString(const std::string& s);

#endif