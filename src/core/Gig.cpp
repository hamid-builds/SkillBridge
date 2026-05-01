#include "core/Gig.h"
#include "core/Exceptions.h"

#include <ostream>
#include <string>

using namespace std;



void Gig::validateTitle(const string& t) const {
    if (t.empty()) {
        throw InvalidGigException("Gig title cannot be empty.");
    }
    if (t.size() < 3) {
        throw InvalidGigException("Gig title must be at least 3 characters.");
    }
    if (t.size() > 100) {
        throw InvalidGigException("Gig title cannot exceed 100 characters.");
    }
}

void Gig::validateDescription(const string& d) const {
    if (d.size() < 10) {
        throw InvalidGigException(
            "Gig description must be at least 10 characters.");
    }
    if (d.size() > 2000) {
        throw InvalidGigException(
            "Gig description cannot exceed 2000 characters.");
    }
}

void Gig::validatePrice(double p) const {
    if (p <= 0.0) {
        throw InvalidGigException("Gig price must be positive.");
    }
    if (p >= 1000000.0) {
        throw InvalidGigException("Gig price must be less than 1,000,000.");
    }
}

void Gig::validateOwnerID(int oid) const {
    if (oid <= 0) {
        throw InvalidGigException("Gig ownerID must be a positive integer.");
    }
}



Gig::Gig()
    : gigID(0),
    ownerID(0),
    title(""),
    description(""),
    price(0.0),
    category(GigCategory::OTHER),
    isActive(true),
    createdAt("") {
 
}

Gig::Gig(int ownerID_,
    const string& title_,
    const string& description_,
    double price_,
    GigCategory category_)
    : gigID(0),
    ownerID(0),
    title(""),
    description(""),
    price(0.0),
    category(GigCategory::OTHER),
    isActive(true),
    createdAt("") {
   
    setOwnerID(ownerID_);
    setTitle(title_);
    setDescription(description_);
    setPrice(price_);
    setCategory(category_);
    
}



void Gig::setOwnerID(int oid) {
    validateOwnerID(oid);
    ownerID = oid;
}

void Gig::setTitle(const string& t) {
    validateTitle(t);
    title = t;
}

void Gig::setDescription(const string& d) {
    validateDescription(d);
    description = d;
}

void Gig::setPrice(double p) {
    validatePrice(p);
    price = p;
}


ostream& operator<<(ostream& os, const Gig& g) {
    os << "[#" << g.gigID << "] \"" << g.title << "\""
        << " by user#" << g.ownerID
        << " | " << gigCategoryToString(g.category)
        << " | Rs " << g.price
        << " | " << (g.isActive ? "active" : "inactive");
    return os;
}