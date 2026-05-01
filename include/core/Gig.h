#ifndef SKILLBRIDGE_GIG_H
#define SKILLBRIDGE_GIG_H

#include <string>
#include <iosfwd> 
#include "GigCategory.h"



class Gig {
private:
    int         gigID;         
    int         ownerID;       
    std::string title;        
    std::string description;   
    double      price;         
    GigCategory category;
    bool        isActive;      
    std::string createdAt;     

   
    void validateTitle(const std::string& t) const;
    void validateDescription(const std::string& d) const;
    void validatePrice(double p) const;
    void validateOwnerID(int oid) const;

public:
  
    Gig();

  
    Gig(int ownerID,
        const std::string& title,
        const std::string& description,
        double price,
        GigCategory category);

   
    int                getGigID()       const
    { return gigID; }
    int                getOwnerID()     const 
    { return ownerID; }
    const std::string& getTitle()       const 
    { return title; }
    const std::string& getDescription() const
    { return description; }
    double             getPrice()       const
    { return price; }
    GigCategory        getCategory()    const
    { return category; }
    bool               getIsActive()    const 
    { return isActive; }
    const std::string& getCreatedAt()   const 
    { return createdAt; }

   
    void setGigID(int id) { gigID = id; }
    void setCreatedAt(const std::string& ts) { createdAt = ts; }
    void setIsActive(bool active) { isActive = active; }

    void setOwnerID(int oid);
    void setTitle(const std::string& t);
    void setDescription(const std::string& d);
    void setPrice(double p);
    void setCategory(GigCategory c) { category = c; }

   
    void deactivate() { isActive = false; }

   

    bool operator==(const Gig& other) const { return gigID == other.gigID; }
    bool operator!=(const Gig& other) const { return !(*this == other); }

   
    bool operator<(const Gig& other) const { return price < other.price; }

    friend std::ostream& operator<<(std::ostream& os, const Gig& g);
};

#endif