#include "core/Exceptions.h"
#include "core/User.h"
#include "core/Client.h"
#include "core/Freelancer.h"
#include "core/Admin.h"
#include "utils/DataList.h"
#include "managers/DatabaseManager.h"
#include "managers/SQLiteUserRepository.h"
#include <iostream>

using namespace std;

int main() 
{
    try 
    {
        DatabaseManager::getInstance().open("db/skillbridge.db");
        cout << "Database open. Schema initialized." << endl;
        
        SQLiteUserRepository repo;

        cout << "\n=== Test 1: Save a Client ===" << endl;
        Client* ayesha = new Client(0, "Ayesha Khan", "ayesha@fast.edu.pk", "sha256hash_ayesha", 500.0);
        bool ok = repo.saveUser(ayesha);
        cout << "Saved: " << (ok ? "yes" : "no") << ", assigned ID: " << ayesha->getUserID() << endl;

        cout << "\n=== Test 2: Save a Freelancer ===" << endl;
        Freelancer* bilal = new Freelancer(0, "Bilal Ahmed", "bilal@fast.edu.pk", "sha256hash_bilal", "UI/UX designer, 2 years experience", "Figma,Photoshop,HTML,CSS", 4.5, 0.0);
        repo.saveUser(bilal);
        cout << "Saved: ID " << bilal->getUserID() << endl;

        cout << "\n=== Test 3: Save an Admin ===" << endl;
        Admin* zainab = new Admin(0, "Zainab Admin", "admin@fast.edu.pk", "sha256hash_admin");
        repo.saveUser(zainab);
        cout << "Saved: ID " << zainab->getUserID() << endl;

        cout << "\n=== Test 4: Duplicate email detection ===" << endl;
        Client* dup = new Client(0, "Duplicate", "ayesha@fast.edu.pk", "hash_dup");
        bool dupOk = repo.saveUser(dup);
        cout << "Duplicate save succeeded: " << (dupOk ? "yes (BUG!)" : "no (correct)") << endl;
        delete dup;

        cout << "\n=== Test 5: emailExists ===" << endl;
        cout << "ayesha@fast.edu.pk: " << repo.emailExists("ayesha@fast.edu.pk") << endl;
        cout << "nobody@nowhere.com: " << repo.emailExists("nobody@nowhere.com") << endl;

        cout << "\n=== Test 6: findUserByEmail with polymorphism ===" << endl;
        User* found = repo.findUserByEmail("bilal@fast.edu.pk");
        if (found) 
        {
            cout << "Found: " << found->getName() << ", role: " << roleToString(found->getRole()) << endl;
            Freelancer* asFreelancer = dynamic_cast<Freelancer*>(found);
            if (asFreelancer) 
            {
                cout << "Portfolio: " << asFreelancer->getPortfolio() << endl;
                cout << "Skills: " << asFreelancer->getSkills() << endl;
                cout << "Rating: " << asFreelancer->getAvgRating() << endl;
            }
            delete found;
        }
        else 
        {
            cout << "Not found (BUG!)" << endl;
        }

        cout << "\n=== Test 7: findAllUsers ===" << endl;
        DataList<User*> all = repo.findAllUsers();
        cout << "Total users: " << all.size() << endl;
        for (int i = 0; i < all.size(); i++) 
        {
            User* u = all[i];
            cout << "  [" << u->getUserID() << "] " << u->getName() << " (" << roleToString(u->getRole()) << ")" << endl;
        }
        
        for (int i = 0; i < all.size(); i++) 
            delete all[i];

        cout << "\n=== Test 8: updateUser ===" << endl;
        ayesha->setName("Ayesha K. Updated");
        ayesha->deposit(250.0);
        ok = repo.updateUser(ayesha);
        cout << "Updated: " << (ok ? "yes" : "no") << endl;

        User* reloaded = repo.findUserByID(ayesha->getUserID());
        cout << "Reloaded name: " << reloaded->getName() << endl;
        cout << "Reloaded balance: " << reloaded->getBalance() << endl;
        delete reloaded;
        cout << "\n=== Test 9: deleteUser ===" << endl;
        int adminID = zainab->getUserID();
        ok = repo.deleteUser(adminID);
        cout << "Deleted admin: " << (ok ? "yes" : "no") << endl;

        User* shouldBeGone = repo.findUserByID(adminID);
        cout << "Lookup after delete: " << (shouldBeGone ? "still exists (BUG!)" : "nullptr (correct)") << endl;

        delete ayesha;
        delete bilal;
        delete zainab;

        DatabaseManager::getInstance().close();
        cout << "\nAll tests complete." << endl;
    }
    catch (const SkillBridgeException& e) 
    {
        cout << "\nERROR: " << e.what() << endl;
        return 1;
    }
    return 0;
}