#ifndef SKILLBRIDGE_API_SERIALIZERS_H
#define SKILLBRIDGE_API_SERIALIZERS_H

#include "json.hpp"

class User;
class Gig;
class Order;
class Review;
class Endorsement;

namespace sb 
{
    namespace api 
    {

        nlohmann::json toJson(const User& u);

        nlohmann::json toJson(const Gig& g);
        nlohmann::json toJson(const Order& o);
        nlohmann::json toJson(const Review& r);
        nlohmann::json toJson(const Endorsement& e);

    }
}

#endif