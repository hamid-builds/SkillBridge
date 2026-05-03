//
//
//#include "managers/SQLiteUserRepository.h"
//#include "managers/SQLiteGigRepository.h"
//#include "managers/SQLiteOrderRepository.h"     
//#include "managers/UserManager.h"
//#include "managers/GigManager.h"
//#include "managers/OrderManager.h"               
//#include "managers/OrderStateMachine.h"          
//#include "managers/CommandHistory.h"             
//#include "managers/TFIDFRanker.h"
//#include "managers/LevenshteinMatcher.h"
//#include "managers/InvertedIndex.h"
//#include "managers/SeedData.h"
//#include "managers/DatabaseManager.h"
//#include "utils/SHA256Hasher.h"
//#include "cli/CLIApp.h"
//#include "core/User.h"
//#include "core/UserRole.h"
//#include "core/Exceptions.h"
//#include "utils/DataList.h"
//
//#include <iostream >
//#include <exception>
//
//using namespace std;
//
//
//static void firstRunSeed(IGigRepository& gigRepo,
//    UserManager& userManager) {
//    DataList<Gig> existing = gigRepo.findAllActiveGigs();
//    if (existing.size() > 0) {
//        return;
//    }
//
//    cout << "First run detected: seeding demo data...\n";
//
//    DataList<int> freelancerIDs;
//
//    const char* demos[][3] = {
//        { "Demo Freelancer One", "demo1@skillbridge.local", "demo1234" },
//        { "Demo Freelancer Two", "demo2@skillbridge.local", "demo1234" },
//    };
//
//    for (int i = 0; i < 2; ++i) {
//        try {
//            User* u = userManager.registerUser(
//                demos[i][0], demos[i][1], demos[i][2],
//                UserRole::FREELANCER);
//            freelancerIDs.add(u->getUserID());
//            userManager.logout();
//        }
//        catch (const DuplicateEntryException&) {
//            User* u = userManager.login(demos[i][1], demos[i][2]);
//            freelancerIDs.add(u->getUserID());
//            userManager.logout();
//        }
//    }
//
//    int saved = SeedData::seedGigs(gigRepo, freelancerIDs);
//    cout << "Seeded " << saved << " gigs across "
//        << freelancerIDs.size() << " demo freelancers.\n";
//}
//
//int main() {
//    try {
//        
//        DatabaseManager::getInstance().open("skillbridge.db");
//
//       
//        SQLiteUserRepository  userRepo;
//        SQLiteGigRepository   gigRepo;
//        SQLiteOrderRepository orderRepo;            
//
//        SHA256Hasher hasher;
//
//        
//        UserManager userManager(&userRepo, &hasher);
//
//        
//        InvertedIndex      sharedIndex;
//        TFIDFRanker        ranker(sharedIndex);
//        LevenshteinMatcher matcher;
//
//        
//        GigManager gigManager(&gigRepo, &userRepo,
//            &ranker, &matcher,
//            sharedIndex);
//
//       
//        OrderStateMachine stateMachine;
//        OrderManager      orderManager(orderRepo, userRepo, gigRepo, stateMachine);
//        CommandHistory    history;
//
//        
//        firstRunSeed(gigRepo, userManager);
//
//        
//        gigManager.rebuildIndexes();
//
//        
//        CLIApp app(userManager, gigManager, orderManager, history);
//
//        
//        app.run();
//    }
//    catch (const SkillBridgeException& e) {
//        cerr << "\nFatal error: " << e.what() << "\n";
//        return 1;
//    }
//    catch (const exception& e) {
//        cerr << "\nUnexpected error: " << e.what() << "\n";
//        return 1;
//    }
//    catch (...) {
//        cerr << "\nUnknown fatal error\n";
//        return 1;
//    }
//
//    return 0;
//}