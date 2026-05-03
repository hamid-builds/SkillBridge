#ifndef SKILLBRIDGE_FAKE_REPOS_H
#define SKILLBRIDGE_FAKE_REPOS_H


#include "managers/IUserRepository.h"
#include "managers/IGigRepository.h"
#include "managers/IOrderRepository.h"
#include "core/User.h"
#include "core/Client.h"
#include "core/Freelancer.h"
#include "core/Admin.h"
#include "core/Gig.h"
#include "core/GigCategory.h"
#include "core/Order.h"
#include "core/Exceptions.h"

inline User* cloneUser(const User* u) {
    if (!u) return nullptr;
    switch (u->getRole()) {
    case UserRole::CLIENT:
        return new Client(u->getUserID(), u->getName(), u->getEmail(),
u->getPasswordHash(), u->getBalance(), u->getCreatedAt());
    case UserRole::FREELANCER: {
       
        const Freelancer* f = dynamic_cast<const Freelancer*>(u);
        std::string portfolio = f ? f->getPortfolio() : "";
        std::string skills = f ? f->getSkills() : "";
        double avgRating = f ? f->getAvgRating() : 0.0;
        return new Freelancer(u->getUserID(), u->getName(), u->getEmail(), u->getPasswordHash(), portfolio,
            skills,avgRating, u->getBalance(), u->getCreatedAt());
    }
    case UserRole::ADMIN:
        return new Admin(u->getUserID(), u->getName(), u->getEmail(), u->getPasswordHash(), u->getBalance(),  u->getCreatedAt());
    }
    return nullptr;
}

class FakeUserRepo : public IUserRepository {
public:
    DataList<User*> users;  


    ~FakeUserRepo() override {
        for (int i = 0; i < users.size(); ++i) delete users.get(i);
    }

    bool saveUser(User*) override { return true; }

    bool updateUser(User* u) override {
      

        for (int i = 0; i < users.size(); ++i) {
            if (users.get(i)->getUserID() == u->getUserID()) {
                User* old = users.get(i);
                User* fresh = cloneUser(u);
                users.removeAt(i);
                users.add(fresh);
                delete old;
                return true;
            }
        }
        return false;
    }

    User* findUserByEmail(const std::string&) override { return nullptr; }

    User* findUserByID(int id) override {
        
        for (int i = 0; i < users.size(); ++i) {
            User* u = users.get(i);
            if (u->getUserID() == id) {
                return cloneUser(u);
            }
        }
        return nullptr;
    }

    DataList<User*> findAllUsers() override { return DataList<User*>(); }
    bool emailExists(const std::string&) override { return false; }
    bool deleteUser(int) override { return true; }

   
    void addUser(User* u) { users.add(u); }
    double getBalanceOf(int userID) {
        for (int i = 0; i < users.size(); ++i) {
            if (users.get(i)->getUserID() == userID)
                return users.get(i)->getBalance();
        }
        return -1.0;
    }
};

class FakeGigRepo : public IGigRepository {
public:
    DataList<Gig> gigs;

    void saveGig(Gig& g) override {
        g.setGigID(gigs.size() + 1);
        gigs.add(g);
    }
    void updateGig(const Gig&) override {}
    void deactivateGig(int) override {}
    bool deleteGig(int) override { return true; }
    Gig findGigByID(int gid) const override {
        for (int i = 0; i < gigs.size(); ++i) {
            if (gigs.get(i).getGigID() == gid) return gigs.get(i);
        }
        throw GigNotFoundException("gigID=" + std::to_string(gid));
    }
    DataList<Gig> findGigsByOwner(int) const override { return DataList<Gig>(); }
    DataList<Gig> findAllActiveGigs() const override { return DataList<Gig>(); }
    DataList<Gig> findAllGigs() const override { return DataList<Gig>(); }

    

    void addGig(int ownerID, double price) {
        Gig g(ownerID, "Test Gig Title",
            "Test gig description text here.",
            price, GigCategory::DESIGN);
        saveGig(g);
    }
    void setInactive(int gigID) {
        for (int i = 0; i < gigs.size(); ++i) {
            if (gigs.get(i).getGigID() == gigID) {
                gigs.get(i).setIsActive(false);
                return;
            }
        }
    }
};

class FakeOrderRepo : public IOrderRepository {
public:
    DataList<Order> orders;
    int nextID = 1;
    bool failNextSave = false;

    void saveOrder(Order& o) override {
        if (failNextSave) {
            failNextSave = false;
            throw DatabaseException("simulated save failure");
        }
        o.setOrderID(nextID++);
        orders.add(o);
    }
    void updateOrder(const Order& o) override 
    {
        for (int i = 0; i < orders.size(); ++i) {
            if (orders.get(i).getOrderID() == o.getOrderID()) {
                orders.removeAt(i);
                orders.add(o);
                return;
            }
        }
        throw OrderNotFoundException("orderID=" + std::to_string(o.getOrderID()));
    }
    
    void updateOrderStatus(int orderID, OrderStatus s, const std::string& c) override 
    {
        for (int i = 0; i < orders.size(); ++i) {
            if (orders.get(i).getOrderID() == orderID) {
                orders.get(i).setStatus(s);
                orders.get(i).setCompletedAt(c);
                return;
            }
        }
        throw OrderNotFoundException("orderID=" + std::to_string(orderID));
    }
    bool deleteOrder(int orderID) override {
        for (int i = 0; i < orders.size(); ++i) {
            if (orders.get(i).getOrderID() == orderID) {
                orders.removeAt(i);
                return true;
            }
        }
        return false;
    }
    Order findOrderByID(int orderID) const override 
    {
        for (int i = 0; i < orders.size(); ++i) 
        {
            if (orders.get(i).getOrderID() == orderID) return orders.get(i);
        }
        throw OrderNotFoundException("orderID=" + std::to_string(orderID));
    }
    DataList<Order> findOrdersByBuyer(int b) const override 
    {
        DataList<Order> r;
        for (int i = 0; i < orders.size(); ++i)
            if (orders.get(i).getBuyerID() == b) 
                r.add(orders.get(i));
        return r;
    }
    DataList<Order> findOrdersBySeller(int s) const override 
    {
        DataList<Order> r;
        for (int i = 0; i < orders.size(); ++i)
            if (orders.get(i).getSellerID() == s) 
                r.add(orders.get(i));
        return r;
    }
    DataList<Order> findOrdersByGig(int g) const override 
    {
        DataList<Order> r;
        for (int i = 0; i < orders.size(); ++i)
            if (orders.get(i).getGigID() == g)
                r.add(orders.get(i));
        return r;
    }
    DataList<Order> findAllOrders() const override 
    {
        DataList<Order> r;
        for (int i = 0; i < orders.size(); ++i) 
            r.add(orders.get(i));
        return r;
    }
};

#endif