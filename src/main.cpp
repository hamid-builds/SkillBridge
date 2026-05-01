#include <iostream>
#include <exception>

#include "managers/DatabaseManager.h"
#include "managers/SQLiteUserRepository.h"
#include "managers/UserManager.h"
#include "utils/SHA256Hasher.h"
#include "cli/CLIApp.h"
#include "core/Exceptions.h"

using namespace std;

int main() 
{
    try {
        DatabaseManager::getInstance().open("db/skillbridge.db");
        SQLiteUserRepository repo;
        SHA256Hasher hasher;
        UserManager userManager(&repo, &hasher);

        CLIApp app(userManager);
        app.run();
        DatabaseManager::getInstance().close();
    }
    catch (const SkillBridgeException& e) 
    {
        cout << "[Fatal] " << e.what() << "\n";
        return 1;
    }
    catch (const exception& e) 
    {
        cout << "[Fatal/Unexpected] " << e.what() << "\n";
        return 1;
    }
    return 0;
}