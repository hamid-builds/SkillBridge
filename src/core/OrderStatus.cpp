#include "core/OrderStatus.h"
#include "core/Exceptions.h"

using namespace std;

string orderStatusToString(OrderStatus status) {
    switch (status) 
    {
    case OrderStatus::PENDING:     
        return "PENDING";
    case OrderStatus::IN_PROGRESS: 
        return "IN_PROGRESS";
    case OrderStatus::COMPLETED:   
        return "COMPLETED";
    case OrderStatus::CANCELLED:  
        return "CANCELLED";
    }
   
    throw ValidationException("orderStatusToString: unknown OrderStatus value");
}

OrderStatus orderStatusFromString(const string& s) {
    if (s == "PENDING")  
        return OrderStatus::PENDING;
    if (s == "IN_PROGRESS")
        return OrderStatus::IN_PROGRESS;
    if (s == "COMPLETED") 
        return OrderStatus::COMPLETED;
    if (s == "CANCELLED")   
        return OrderStatus::CANCELLED;
    throw ValidationException("orderStatusFromString: unknown status '" + s + "'");
}