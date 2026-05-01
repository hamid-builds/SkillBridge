#include <iostream>
#include <string>
#include <cstdio>  

#include "managers/DatabaseManager.h"
#include "managers/SQLiteGigRepository.h"
#include "managers/SQLiteUserRepository.h"
#include "managers/UserFactory.h"
#include "utils/SHA256Hasher.h"


#include "core/Gig.h"
#include "core/GigCategory.h"
#include "core/Exceptions.h"

using namespace std;

static int g_pass = 0;
static int g_fail = 0;

static void check(bool condition, const string& label) {
    if (condition) {
        cout << "  PASS  " << label << "\n";
        g_pass++;
    }
    else {
        cout << "  FAIL  " << label << "\n";
        g_fail++;
    }
}


template <typename ExType, typename Fn>
static void expectThrow(Fn fn, const string& label) {
    try {
        fn();
        cout << "  FAIL  " << label << " (no exception)\n";
        g_fail++;
    }
    catch (const ExType&) {
        cout << "  PASS  " << label << "\n";
        g_pass++;
    }
    catch (const exception& e) {
        cout << "  FAIL  " << label << " (wrong type: " << e.what() << ")\n";

        g_fail++;
    }
}



static int seedFreelancer(int n) {
    SQLiteUserRepository userRepo;
    SHA256Hasher hasher;

    string email = "freelancer" + to_string(n) + "@test.com";
    User* u = UserFactory::create(
        UserRole::FREELANCER,
        "Freelancer " + to_string(n),
        email,
        hasher.hash("password123"));

    userRepo.saveUser(u);
    int id = u->getUserID();
    delete u;
    return id;
}



static void testSaveAssignsID(IGigRepository* repo, int ownerID) {
    cout << "\n[saveGig assigns DB-generated ID and createdAt]\n";

    Gig g(ownerID, "Logo Design",
        "Professional logo design for startups and personal brands.",
        500.0, GigCategory::DESIGN);

    check(g.getGigID() == 0, "gigID is 0 before save");
    check(g.getCreatedAt().empty(), "createdAt empty before save");

    repo->saveGig(g);

    check(g.getGigID() > 0, "gigID assigned after save");
    check(!g.getCreatedAt().empty(), "createdAt populated after save");
}

static void testFindByID(IGigRepository* repo, int ownerID) {
    cout << "\n[findGigByID: hit and miss]\n";

    Gig g(ownerID, "Resume Writing",
        "I will write and polish a professional resume for you.",
        300.0, GigCategory::WRITING);
    repo->saveGig(g);

    Gig fetched = repo->findGigByID(g.getGigID());
    check(fetched.getTitle() == "Resume Writing",
        "findGigByID returns correct gig");
    check(fetched.getOwnerID() == ownerID,
        "ownerID preserved through round-trip");
    check(fetched.getCategory() == GigCategory::WRITING,
        "category preserved through round-trip");
    check(fetched.getIsActive() == true,
        "isActive preserved through round-trip");

    expectThrow<GigNotFoundException>(
        [&]() { repo->findGigByID(99999); },
        "findGigByID throws on unknown ID");
}

static void testUpdate(IGigRepository* repo, int ownerID) {
    cout << "\n[updateGig: modify existing + reject missing]\n";

    Gig g(ownerID, "Math Tutoring",
        "One-hour tutoring session for high school math.",
        200.0, GigCategory::TUTORING);
    repo->saveGig(g);

    g.setPrice(250.0);
    g.setTitle("Math Tutoring Premium");
    repo->updateGig(g);

    Gig reloaded = repo->findGigByID(g.getGigID());
    check(reloaded.getPrice() == 250.0, "updateGig changed price");
    check(reloaded.getTitle() == "Math Tutoring Premium",
        "updateGig changed title");

    Gig ghost(ownerID, "Ghost Gig",
        "A gig that was never saved to the repo.",
        100.0, GigCategory::OTHER);
    ghost.setGigID(77777);

    expectThrow<GigNotFoundException>(
        [&]() { repo->updateGig(ghost); },
        "updateGig throws on missing gig");
}

static void testDeactivate(IGigRepository* repo, int ownerID) {
    cout << "\n[deactivateGig: soft delete]\n";

    Gig g(ownerID, "SEO Audit",
        "I will audit your website and deliver an SEO report.",
        400.0, GigCategory::MARKETING);
    repo->saveGig(g);
    int id = g.getGigID();

    check(repo->findGigByID(id).getIsActive() == true, "new gig is active");
    repo->deactivateGig(id);
    check(repo->findGigByID(id).getIsActive() == false,
        "deactivateGig sets isActive=false");

    expectThrow<GigNotFoundException>(
        [&]() { repo->deactivateGig(88888); },
        "deactivateGig throws on missing gig");
}

static void testDeleteGig(IGigRepository* repo, int ownerID) {
    cout << "\n[deleteGig: hard delete]\n";

    Gig g(ownerID, "Quick Task",
        "Short-lived gig for demonstrating hard delete.",
        50.0, GigCategory::OTHER);
    repo->saveGig(g);
    int id = g.getGigID();

    bool removed = repo->deleteGig(id);
    check(removed == true, "deleteGig returns true when gig existed");

    expectThrow<GigNotFoundException>(
        [&]() { repo->findGigByID(id); },
        "deleted gig is no longer findable");

    bool removedAgain = repo->deleteGig(id);
    check(removedAgain == false,
        "deleteGig returns false when gig was already gone");
}

static void testBulkFinders(IGigRepository* repo) {
    cout << "\n[bulk finders: by-owner, all-active, all]\n";

    
    int ownerA = seedFreelancer(100);
    int ownerB = seedFreelancer(101);

    Gig a(ownerA, "Gig A", "Description A valid length here.",
        100.0, GigCategory::DESIGN);
    Gig b(ownerA, "Gig B", "Description B valid length here.",
        200.0, GigCategory::CODING);
    Gig c(ownerB, "Gig C", "Description C valid length here.",
        300.0, GigCategory::WRITING);
    repo->saveGig(a);
    repo->saveGig(b);
    repo->saveGig(c);

    DataList<Gig> ownedByA = repo->findGigsByOwner(ownerA);
    check(ownedByA.size() == 2, "findGigsByOwner returns 2 for ownerA");

    DataList<Gig> none = repo->findGigsByOwner(999999);
    check(none.size() == 0, "findGigsByOwner returns empty for unknown owner");

  
    DataList<Gig> activeAll = repo->findAllActiveGigs();
    DataList<Gig> everyGig = repo->findAllGigs();

    check(activeAll.size() > 0, "findAllActiveGigs returns nonzero");
    check(everyGig.size() >= activeAll.size(),
        "findAllGigs count >= findAllActiveGigs count");
}

static void testForeignKey(IGigRepository* repo) {
    cout << "\n[foreign key: ownerID must reference real user]\n";

    Gig g(999999,  
        "Ghost Owner Gig",
        "This gig's owner does not exist in the users table.",
        100.0, GigCategory::OTHER);

    
    expectThrow<ValidationException>(
        [&]() { repo->saveGig(g); },
        "saveGig rejects gig with nonexistent ownerID");
}

static void testCascadeDelete(IGigRepository* repo) {
    cout << "\n[cascade: deleting a user removes their gigs]\n";

   
    int uid = seedFreelancer(200);
    Gig g(uid, "Cascade Test Gig",
        "This gig should vanish when its owner is deleted.",
        100.0, GigCategory::OTHER);
    repo->saveGig(g);
    int gigID = g.getGigID();

    
    check(repo->findGigByID(gigID).getGigID() == gigID,
        "gig exists before owner deletion");

    
    SQLiteUserRepository userRepo;
    userRepo.deleteUser(uid);

   
    expectThrow<GigNotFoundException>(
        [&]() { repo->findGigByID(gigID); },
        "gig is auto-deleted when owner is deleted");
}



int main() {
    cout << "=======================================\n";
    cout << " Module 2 Step 4: SQLiteGigRepository\n";
    cout << "=======================================\n";

    
    const string TEST_DB = "db/test_gig_repo.db";

   
    remove(TEST_DB.c_str());

    try {
        DatabaseManager::getInstance().open(TEST_DB);

        SQLiteGigRepository concrete;
        IGigRepository* repo = &concrete;

        int owner1 = seedFreelancer(1);

        testSaveAssignsID(repo, owner1);
        testFindByID(repo, owner1);
        testUpdate(repo, owner1);
        testDeactivate(repo, owner1);
        testDeleteGig(repo, owner1);
        testBulkFinders(repo);
        testForeignKey(repo);
        testCascadeDelete(repo);

        DatabaseManager::getInstance().close();
    }
    catch (const exception& e) {
        cout << "\nUNEXPECTED EXCEPTION AT TOP LEVEL: " << e.what() << "\n";
        g_fail++;
    }

    cout << "\n---------------------------------------\n";
    cout << "Passed: " << g_pass << "  Failed: " << g_fail << "\n";
    cout << "---------------------------------------\n";

    return g_fail == 0 ? 0 : 1;
}