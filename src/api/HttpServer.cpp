#include "api/HttpServer.h"

#include "httplib.h"
#include "json.hpp"

#include "managers/UserManager.h"
#include "managers/DatabaseManager.h"
#include "core/User.h"
#include "core/UserRole.h"
#include "core/Exceptions.h"
#include "api/Serializers.h"

#include <iostream>
#include <string>
#include <unordered_map>
#include <mutex>
#include <random>
#include <sstream>
#include <iomanip>


namespace sb 
{
    namespace api 
    {

        struct HttpServer::Impl 
        {
            httplib::Server server;
            std::unordered_map<std::string, int> sessions;
            std::mutex sessionMutex;
        };

        namespace 
        {

            std::string generateSessionToken() 
            {
                static thread_local std::random_device rd;
                static thread_local std::mt19937_64 gen(rd());
                std::uniform_int_distribution<uint64_t> dist;

                uint64_t hi = dist(gen);
                uint64_t lo = dist(gen);

                std::ostringstream oss;
                oss << std::hex << std::setfill('0') << std::setw(16) << hi << std::setw(16) << lo;
                return oss.str();
            }

            std::string extractBearerToken(const httplib::Request& req) 
            {
                if (!req.has_header("Authorization")) 
                    return "";
                std::string auth = req.get_header_value("Authorization");
                const std::string prefix = "Bearer ";
                if (auth.size() <= prefix.size()) 
                    return "";
                if (auth.compare(0, prefix.size(), prefix) != 0) 
                    return "";
                return auth.substr(prefix.size());
            }
            void sendError(httplib::Response& res, int status, const std::string& msg) 
            {
                nlohmann::json j;
                j["error"] = msg;
                res.status = status;
                res.set_content(j.dump(), "application/json");
            }

            void sendJson(httplib::Response& res, const nlohmann::json& body, int status = 200) 
            {
                res.status = status;
                res.set_content(body.dump(), "application/json");
            }

        }

        HttpServer::HttpServer(const AppContext& ctx) : impl_(new Impl()), ctx_(ctx)
        {

        }

        HttpServer::~HttpServer() 
        {
            delete impl_;
        }

        bool HttpServer::start(int port) 
        {
            impl_->server.set_default_headers({
                {"Access-Control-Allow-Origin",  "*"},
                {"Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS"},
                {"Access-Control-Allow-Headers", "Content-Type"}
                });

            impl_->server.Options(R"(.*)",
                [](const httplib::Request&, httplib::Response& res) 
                {
                    res.status = 204;
                });
            registerHealthRoutes();
            registerAuthRoutes();
            registerStaticFiles();

            std::cout << "[HttpServer] Listening on http://localhost:" << port << "\n";
            std::cout << "[HttpServer] Press Ctrl+C to stop the server.\n";

            bool ok = impl_->server.listen("localhost", port);
            if (!ok) 
            {
                std::cerr << "[HttpServer] Failed to bind to localhost:" << port << ". Is another process using this port?\n";
                return false;
            }
            return true;
        }

        void HttpServer::stop() 
        {
            impl_->server.stop();
        }

        void HttpServer::registerHealthRoutes() 
        {
            impl_->server.Get("/api/health",
                [](const httplib::Request&, httplib::Response& res) 
                {
                    nlohmann::json j;
                    j["status"] = "ok";
                    j["mode"] = "web";
                    j["service"] = "SkillBridge";
                    res.set_content(j.dump(), "application/json");
                });
        }

        void HttpServer::registerStaticFiles() 
        {
            
            bool mounted = impl_->server.set_mount_point("/", "./public");
            if (!mounted) 
            {
                std::cerr << "[HttpServer] Warning: ./public/ not found. "
                    << "Static files will not be served until you create that "
                    << "directory next to the executable.\n";
            }
        }

        void HttpServer::registerAuthRoutes() 
        {

            impl_->server.Post("/api/register",
                [this](const httplib::Request& req, httplib::Response& res) 
                {
                    try 
                    {
                        auto body = nlohmann::json::parse(req.body);

                        std::string name = body.value("name", "");
                        std::string email = body.value("email", "");
                        std::string password = body.value("password", "");
                        std::string roleStr = body.value("role", "");

                        UserRole role;
                        try 
                        {
                            role = stringToRole(roleStr);
                        }
                        catch (const ValidationException&) 
                        {
                            sendError(res, 400, "invalid role for registration");
                            return;
                        }

                        std::lock_guard<std::mutex> dbLock(DatabaseManager::getInstance().getWriteMutex());

                        User* user = ctx_.userManager->registerUser(name, email, password, role);

                        int userID = user->getUserID();
                        nlohmann::json userJson = sb::api::toJson(*user);
                        ctx_.userManager->logout();

                        std::string token = generateSessionToken();
                        {
                            std::lock_guard<std::mutex> sessLock(impl_->sessionMutex);
                            impl_->sessions[token] = userID;
                        }

                        nlohmann::json out;
                        out["token"] = token;
                        out["user"] = userJson;
                        sendJson(res, out, 201);
                    }
                    catch (const ValidationException& e) 
                    {
                        sendError(res, 400, e.what());
                    }
                    catch (const DuplicateEntryException& e) 
                    {
                        sendError(res, 409, e.what());
                    }
                    catch (const RateLimitException& e) 
                    {
                        sendError(res, 429, e.what());
                    }
                    catch (const SkillBridgeException& e) 
                    {
                        sendError(res, 500, e.what());
                    }
                    catch (const nlohmann::json::exception&) 
                    {
                        sendError(res, 400, "invalid JSON in request body");
                    }
                    catch (const std::exception& e) 
                    {
                        sendError(res, 500, std::string("unexpected error: ") + e.what());
                    }
                });

            impl_->server.Post("/api/login",
                [this](const httplib::Request& req, httplib::Response& res) 
                {
                    User* user = nullptr;
                    try {
                        auto body = nlohmann::json::parse(req.body);
                        std::string email = body.value("email", "");
                        std::string password = body.value("password", "");

                        user = ctx_.userManager->authenticate(email, password);

                        std::string token = generateSessionToken();
                        {
                            std::lock_guard<std::mutex> sessLock(impl_->sessionMutex);
                            impl_->sessions[token] = user->getUserID();
                        }

                        nlohmann::json out;
                        out["token"] = token;
                        out["user"] = sb::api::toJson(*user);

                        delete user;
                        sendJson(res, out, 200);
                    }
                    catch (const ValidationException& e) 
                    {
                        delete user;
                        sendError(res, 400, e.what());
                    }
                    catch (const AuthenticationException& e) 
                    {
                        delete user;
                        sendError(res, 401, e.what());
                    }
                    catch (const RateLimitException& e) 
                    {
                        delete user;
                        sendError(res, 429, e.what());
                    }
                    catch (const SkillBridgeException& e) 
                    {
                        delete user;
                        sendError(res, 500, e.what());
                    }
                    catch (const nlohmann::json::exception&) 
                    {
                        delete user;
                        sendError(res, 400, "invalid JSON in request body");
                    }
                    catch (const std::exception& e) 
                    {
                        delete user;
                        sendError(res, 500, std::string("unexpected error: ") + e.what());
                    }
                });

            impl_->server.Post("/api/logout",
                [this](const httplib::Request& req, httplib::Response& res) 
                {
                    std::string token = extractBearerToken(req);
                    if (!token.empty()) 
                    {
                        std::lock_guard<std::mutex> sessLock(impl_->sessionMutex);
                        impl_->sessions.erase(token);
                    }

                    nlohmann::json out;
                    out["ok"] = true;
                    sendJson(res, out, 200);
                });

            impl_->server.Get("/api/me",
                [this](const httplib::Request& req, httplib::Response& res) 
                {
                    std::string token = extractBearerToken(req);
                    if (token.empty()) 
                    {
                        sendError(res, 401, "not authenticated");
                        return;
                    }

                    int userID = -1;
                    {
                        std::lock_guard<std::mutex> sessLock(impl_->sessionMutex);
                        auto it = impl_->sessions.find(token);
                        if (it == impl_->sessions.end()) 
                        {
                            sendError(res, 401, "session expired or invalid");
                            return;
                        }
                        userID = it->second;
                    }

                    User* user = nullptr;
                    try 
                    {
                        user = ctx_.userManager->findUserByID(userID);
                        if (!user) 
                        {
                            {
                                std::lock_guard<std::mutex> sessLock(impl_->sessionMutex);
                                impl_->sessions.erase(token);
                            }
                            sendError(res, 401, "user no longer exists");
                            return;
                        }

                        nlohmann::json out;
                        out["user"] = sb::api::toJson(*user);
                        delete user;
                        sendJson(res, out, 200);
                    }
                    catch (const std::exception& e) 
                    {
                        delete user;
                        sendError(res, 500, std::string("unexpected error: ") + e.what());
                    }
                });
        }

    }
}