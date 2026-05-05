#include "managers/SQLiteUserRepository.h"
#include "managers/SQLiteGigRepository.h"
#include "managers/SQLiteOrderRepository.h"
#include "managers/SQLiteMessageRepository.h"
#include "managers/SQLiteReviewRepository.h"
#include "managers/SQLiteEndorsementRepository.h"

#include "managers/UserManager.h"
#include "managers/GigManager.h"
#include "managers/OrderManager.h"
#include "managers/OrderStateMachine.h"
#include "managers/CommandHistory.h"
#include "managers/MessageManager.h"
#include "managers/ReviewManager.h"
#include "managers/SkillGraphManager.h"

#include "managers/TFIDFRanker.h"
#include "managers/LevenshteinMatcher.h"
#include "managers/InvertedIndex.h"
#include "managers/SeedData.h"
#include "managers/DatabaseManager.h"

#include "strategies/BagOfWordsSentiment.h"

#include "utils/SHA256Hasher.h"
#include "utils/HuffmanCoder.h"

#include "cli/CLIApp.h"

#include "core/User.h"
#include "core/UserRole.h"
#include "core/Exceptions.h"

#include "utils/DataList.h"

#include <iostream>
#include <exception>

#include "api/HttpServer.h"

using namespace std;

static void firstRunSeed(UserManager& userManager) 
{
    bool seeded = userManager.seedAdmin("Admin", "admin@skillbridge.com", "Admin@4321");

    if (seeded) 
    {
        cout << "First run detected: seeded default admin account.\n";
        cout << "  Email:    admin@skillbridge.com\n";
        cout << "  Password: Admin@4321\n";
        cout << "  (See README for details. Change this password in production.)\n";
    }
}

int main(int argc, char* argv[]) 
{
    bool cliMode = false;
    for (int i = 1; i < argc; ++i) 
    {
        if (std::string(argv[i]) == "--cli") 
        {
            cliMode = true;
            break;
        }
    }

    try 
    {
        DatabaseManager::getInstance().open("skillbridge.db");

        sqlite3* conn = DatabaseManager::getInstance().getConnection();

        SQLiteUserRepository        userRepo;
        SQLiteGigRepository         gigRepo;
        SQLiteOrderRepository       orderRepo;
        SQLiteMessageRepository     messageRepo;
        SQLiteReviewRepository      reviewRepo(conn);
        SQLiteEndorsementRepository endorsementRepo(conn);

        SHA256Hasher hasher;

        UserManager userManager(&userRepo, &hasher);

        InvertedIndex      sharedIndex;
        TFIDFRanker        ranker(sharedIndex);
        LevenshteinMatcher matcher;

        GigManager gigManager(&gigRepo, &userRepo, &ranker, &matcher, sharedIndex);

        OrderStateMachine stateMachine;
        OrderManager      orderManager(orderRepo, userRepo, gigRepo, stateMachine);
        CommandHistory    history;

        HuffmanCoder    huffman;
        MessageManager  messageManager(messageRepo, userRepo, huffman);

        BagOfWordsSentiment sentiment;
        ReviewManager reviewManager(reviewRepo, orderRepo, userRepo, sentiment);

        SkillGraphManager skillGraphManager(endorsementRepo, userRepo);

        firstRunSeed(userManager);

        gigManager.rebuildIndexes();

        if (cliMode) 
        {
            cout << "[main] Starting in CLI mode.\n";
            CLIApp app(userManager, gigManager, orderManager, history, messageManager, reviewManager, skillGraphManager);
            app.run();
        }
        else 
        {
            cout << "[main] Starting in web mode.\n";
            cout << "[main] Pass --cli on the command line to use the CLI instead.\n";

            sb::api::AppContext ctx;
            ctx.userManager = &userManager;
            ctx.gigManager = &gigManager;
            ctx.orderManager = &orderManager;
            ctx.commandHistory = &history;
            ctx.messageManager = &messageManager;
            ctx.reviewManager = &reviewManager;
            ctx.skillGraphManager = &skillGraphManager;

            sb::api::HttpServer server(ctx);
            std::system("start http://localhost:8080");

            server.start(8080);
        }
    }
    catch (const SkillBridgeException& e) 
    {
        cerr << "\nFatal error: " << e.what() << "\n";
        return 1;
    }
    catch (const exception& e) 
    {
        cerr << "\nUnexpected error: " << e.what() << "\n";
        return 1;
    }
    catch (...) 
    {
        cerr << "\nUnknown fatal error\n";
        return 1;
    }
    return 0;
}