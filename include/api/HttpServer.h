#ifndef SKILLBRIDGE_API_HTTP_SERVER_H
#define SKILLBRIDGE_API_HTTP_SERVER_H

#include <string>

class UserManager;
class GigManager;
class OrderManager;
class CommandHistory;
class MessageManager;
class ReviewManager;
class SkillGraphManager;

namespace sb 
{
    namespace api 
    {

        struct AppContext 
        {
            UserManager* userManager;
            GigManager* gigManager;
            OrderManager* orderManager;
            CommandHistory* commandHistory;
            MessageManager* messageManager;
            ReviewManager* reviewManager;
            SkillGraphManager* skillGraphManager;
        };

        class HttpServer 
        {
        public:
            explicit HttpServer(const AppContext& ctx);
            ~HttpServer();

            HttpServer(const HttpServer&) = delete;
            HttpServer& operator=(const HttpServer&) = delete;

            bool start(int port = 8080);

            void stop();

        private:
            struct Impl;
            Impl* impl_;

            const AppContext& ctx_;

            void registerHealthRoutes();
            void registerStaticFiles();
            void registerAuthRoutes();
        };

    }
}

#endif