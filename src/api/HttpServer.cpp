// -----------------------------------------------------------------------------
// HttpServer.cpp
//
// pImpl-backed wrapper around cpp-httplib. Configures routes, serves static
// files, and runs the listen loop.
//
// httplib.h is included here ONLY. The header keeps it out of users'
// translation units, which keeps build times sane.
// -----------------------------------------------------------------------------

#include "api/HttpServer.h"

#include "httplib.h"
#include "json.hpp"

#include "managers/OrderManager.h"
#include "managers/UserManager.h"
#include "managers/GigManager.h"
#include "managers/MessageManager.h"
#include "managers/ReviewManager.h"
#include "managers/SkillGraphManager.h"
#include "managers/DatabaseManager.h"
#include "core/Order.h"
#include "core/OrderStatus.h"
#include "core/User.h"
#include "core/Freelancer.h"
#include "core/UserRole.h"
#include "core/Gig.h"
#include "core/GigCategory.h"
#include "core/GigBrowseFilter.h"
#include "core/Review.h"
#include "core/Exceptions.h"
#include "api/Serializers.h"

#include <iostream>
#include <string>
#include <unordered_map>
#include <mutex>
#include <random>
#include <sstream>
#include <iomanip>
#include <memory>
#include <algorithm>


namespace sb {
    namespace api {

        struct HttpServer::Impl {
            httplib::Server server;
            std::unordered_map<std::string, int> sessions;
            std::mutex sessionMutex;
        };

        namespace {

            std::string generateSessionToken() {
                static thread_local std::random_device rd;
                static thread_local std::mt19937_64 gen(rd());
                std::uniform_int_distribution<uint64_t> dist;

                uint64_t hi = dist(gen);
                uint64_t lo = dist(gen);

                std::ostringstream oss;
                oss << std::hex << std::setfill('0')
                    << std::setw(16) << hi
                    << std::setw(16) << lo;
                return oss.str();
            }

            std::string extractBearerToken(const httplib::Request& req) {
                if (!req.has_header("Authorization")) return "";
                std::string auth = req.get_header_value("Authorization");
                const std::string prefix = "Bearer ";
                if (auth.size() <= prefix.size()) return "";
                if (auth.compare(0, prefix.size(), prefix) != 0) return "";
                return auth.substr(prefix.size());
            }

            void sendError(httplib::Response& res, int status, const std::string& msg) {
                nlohmann::json j;
                j["error"] = msg;
                res.status = status;
                res.set_content(j.dump(), "application/json");
            }

            void sendJson(httplib::Response& res, const nlohmann::json& body, int status = 200) {
                res.status = status;
                res.set_content(body.dump(), "application/json");
            }

            GigBrowseFilter parseBrowseFilter(const httplib::Request& req) {
                GigBrowseFilter f;
                if (!req.has_param("category")) return f;
                std::string val = req.get_param_value("category");
                if (val.empty()) return f;
                f.hasCategory = true;
                f.category = gigCategoryFromString(val);
                return f;
            }
            GigSortOrder parseSortOrder(const httplib::Request& req) {
                if (!req.has_param("sort")) return GigSortOrder::NEWEST_FIRST;
                std::string val = req.get_param_value("sort");
                if (val.empty() || val == "newest") return GigSortOrder::NEWEST_FIRST;
                if (val == "price_asc") return GigSortOrder::PRICE_ASC;
                if (val == "price_desc") return GigSortOrder::PRICE_DESC;
                throw std::invalid_argument("Unknown sort: " + val);
            }

            void lookupFreelancerForCard(UserManager* userManager,
                int ownerID,
                std::string& outName,
                double& outRating) {
                outName = "Unknown";
                outRating = 0.0;
                std::unique_ptr<User> owner(userManager->findUserByID(ownerID));
                if (!owner) return;
                outName = owner->getName();
                if (auto* f = dynamic_cast<Freelancer*>(owner.get())) {
                    outRating = f->getAvgRating();
                }
            }

            bool parsePositiveInt(const std::string& s, int& out) {
                if (s.empty()) return false;
                try {
                    size_t pos = 0;
                    long long v = std::stoll(s, &pos);
                    if (pos != s.size()) return false; 
                    if (v <= 0 || v > 2147483647LL) return false;
                    out = static_cast<int>(v);
                    return true;
                }
                catch (...) {
                    return false;
                }
            }

        }
        int HttpServer::requireAuth(const httplib::Request& req,
            httplib::Response& res) {
            std::string token = extractBearerToken(req);
            if (token.empty()) {
                sendError(res, 401, "not authenticated");
                return -1;
            }

            std::lock_guard<std::mutex> sessLock(impl_->sessionMutex);
            auto it = impl_->sessions.find(token);
            if (it == impl_->sessions.end()) {
                sendError(res, 401, "session expired or invalid");
                return -1;
            }
            return it->second;
        }

        HttpServer::HttpServer(const AppContext& ctx)
            : impl_(new Impl()), ctx_(ctx)
        {
        }

        HttpServer::~HttpServer() {
            delete impl_;
        }

        bool HttpServer::start(int port) {
            impl_->server.set_default_headers({
                {"Access-Control-Allow-Origin",  "*"},
                {"Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS"},
                {"Access-Control-Allow-Headers", "Content-Type, Authorization"}
                });

            impl_->server.Options(R"(.*)",
                [](const httplib::Request&, httplib::Response& res) {
                    res.status = 204;
                });

            registerHealthRoutes();
            registerAuthRoutes();
            registerGigRoutes();
            registerReviewRoutes();
            registerUserRoutes();
            registerOrderRoutes();
            registerMessageRoutes();
            registerEndorsementRoutes();
            registerAdminRoutes();
            registerStaticFiles();

            std::cout << "[HttpServer] Listening on http://localhost:" << port << "\n";
            std::cout << "[HttpServer] Press Ctrl+C to stop the server.\n";

            bool ok = impl_->server.listen("localhost", port);
            if (!ok) {
                std::cerr << "[HttpServer] Failed to bind to localhost:" << port
                    << ". Is another process using this port?\n";
                return false;
            }
            return true;
        }

        void HttpServer::stop() {
            impl_->server.stop();
        }

        void HttpServer::registerHealthRoutes() {
            impl_->server.Get("/api/health",
                [](const httplib::Request&, httplib::Response& res) {
                    nlohmann::json j;
                    j["status"] = "ok";
                    j["mode"] = "web";
                    j["service"] = "SkillBridge";
                    res.set_content(j.dump(), "application/json");
                });
        }

        void HttpServer::registerStaticFiles() {
            bool mounted = impl_->server.set_mount_point("/", "./public");
            if (!mounted) {
                std::cerr << "[HttpServer] Warning: ./public/ not found. "
                    << "Static files will not be served until you create that "
                    << "directory next to the executable.\n";
            }
        }

        void HttpServer::registerAuthRoutes() {

            // ----- POST /api/register --------------------------------------------
            impl_->server.Post("/api/register",
                [this](const httplib::Request& req, httplib::Response& res) {
                    try {
                        auto body = nlohmann::json::parse(req.body);

                        std::string name = body.value("name", "");
                        std::string email = body.value("email", "");
                        std::string password = body.value("password", "");
                        std::string roleStr = body.value("role", "");

                        UserRole role;
                        try {
                            role = stringToRole(roleStr);
                        }
                        catch (const ValidationException&) {
                            sendError(res, 400, "invalid role for registration");
                            return;
                        }

                        std::lock_guard<std::mutex> dbLock(
                            DatabaseManager::getInstance().getWriteMutex());

                        User* user = ctx_.userManager->registerUser(
                            name, email, password, role);

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
                    catch (const ValidationException& e) {
                        sendError(res, 400, e.what());
                    }
                    catch (const DuplicateEntryException& e) {
                        sendError(res, 409, e.what());
                    }
                    catch (const RateLimitException& e) {
                        sendError(res, 429, e.what());
                    }
                    catch (const SkillBridgeException& e) {
                        sendError(res, 500, e.what());
                    }
                    catch (const nlohmann::json::exception&) {
                        sendError(res, 400, "invalid JSON in request body");
                    }
                    catch (const std::exception& e) {
                        sendError(res, 500, std::string("unexpected error: ") + e.what());
                    }
                });

            // ----- POST /api/login -----------------------------------------------
            impl_->server.Post("/api/login",
                [this](const httplib::Request& req, httplib::Response& res) {
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
                    catch (const ValidationException& e) {
                        delete user;
                        sendError(res, 400, e.what());
                    }
                    catch (const AuthenticationException& e) {
                        delete user;
                        sendError(res, 401, e.what());
                    }
                    catch (const RateLimitException& e) {
                        delete user;
                        sendError(res, 429, e.what());
                    }
                    catch (const SkillBridgeException& e) {
                        delete user;
                        sendError(res, 500, e.what());
                    }
                    catch (const nlohmann::json::exception&) {
                        delete user;
                        sendError(res, 400, "invalid JSON in request body");
                    }
                    catch (const std::exception& e) {
                        delete user;
                        sendError(res, 500, std::string("unexpected error: ") + e.what());
                    }
                });

            // ----- POST /api/logout ----------------------------------------------
            impl_->server.Post("/api/logout",
                [this](const httplib::Request& req, httplib::Response& res) {
                    std::string token = extractBearerToken(req);
                    if (!token.empty()) {
                        std::lock_guard<std::mutex> sessLock(impl_->sessionMutex);
                        impl_->sessions.erase(token);
                    }

                    nlohmann::json out;
                    out["ok"] = true;
                    sendJson(res, out, 200);
                });

            impl_->server.Get("/api/me",
                [this](const httplib::Request& req, httplib::Response& res) {
                    int userID = requireAuth(req, res);
                    if (userID < 0) return;

                    std::unique_ptr<User> user;
                    try {
                        user.reset(ctx_.userManager->findUserByID(userID));
                        if (!user) {
                            std::string token = extractBearerToken(req);
                            {
                                std::lock_guard<std::mutex> sessLock(impl_->sessionMutex);
                                impl_->sessions.erase(token);
                            }
                            sendError(res, 401, "user no longer exists");
                            return;
                        }

                        nlohmann::json out;
                        out["user"] = sb::api::toJson(*user);
                        sendJson(res, out, 200);
                    }
                    catch (const std::exception& e) {
                        sendError(res, 500, std::string("unexpected error: ") + e.what());
                    }
                });
        }

        void HttpServer::registerGigRoutes() {

            impl_->server.Get("/api/gigs/browse",
                [this](const httplib::Request& req, httplib::Response& res) {
                    int userID = requireAuth(req, res);
                    if (userID < 0) return;

                    try {
                        GigBrowseFilter filter = parseBrowseFilter(req);
                        GigSortOrder sort = parseSortOrder(req);

                        DataList<Gig> gigs =
                            ctx_.gigManager->findActiveGigsForBrowse(filter, sort);

                        nlohmann::json arr = nlohmann::json::array();
                        for (int i = 0; i < gigs.size(); ++i) {
                            const Gig& g = gigs.get(i);

                            std::string fName;
                            double fRating;
                            lookupFreelancerForCard(
                                ctx_.userManager, g.getOwnerID(),
                                fName, fRating);

                            arr.push_back(
                                sb::api::toBrowseCardJson(g, fName, fRating));
                        }

                        nlohmann::json out;
                        out["gigs"] = arr;
                        out["count"] = gigs.size();
                        sendJson(res, out, 200);
                    }
                    catch (const std::invalid_argument& e) {
                        sendError(res, 400, e.what());
                    }
                    catch (const SkillBridgeException& e) {
                        sendError(res, 500, e.what());
                    }
                    catch (const std::exception& e) {
                        sendError(res, 500,
                            std::string("unexpected error: ") + e.what());
                    }
                });

            // ----- GET /api/gigs/search ------------------------------------------
            impl_->server.Get("/api/gigs/search",
                [this](const httplib::Request& req, httplib::Response& res) {
                    int userID = requireAuth(req, res);
                    if (userID < 0) return;

                    try {
                        std::string q;
                        if (req.has_param("q")) q = req.get_param_value("q");
                        size_t start = q.find_first_not_of(" \t\r\n");
                        size_t end = q.find_last_not_of(" \t\r\n");
                        if (start == std::string::npos) {
                            sendError(res, 400, "search query cannot be empty");
                            return;
                        }
                        q = q.substr(start, end - start + 1);

                        int maxResults = 20;
                        if (req.has_param("max")) {
                            try {
                                maxResults = std::stoi(req.get_param_value("max"));
                            }
                            catch (...) {
                                maxResults = 20;
                            }
                        }
                        if (maxResults < 1) maxResults = 1;
                        if (maxResults > 50) maxResults = 50;

                        DataList<Gig> results =
                            ctx_.gigManager->searchGigs(q, maxResults);

                        nlohmann::json arr = nlohmann::json::array();
                        for (int i = 0; i < results.size(); ++i) {
                            const Gig& g = results.get(i);

                            std::string fName;
                            double fRating;
                            lookupFreelancerForCard(
                                ctx_.userManager, g.getOwnerID(),
                                fName, fRating);

                            arr.push_back(
                                sb::api::toBrowseCardJson(g, fName, fRating));
                        }

                        nlohmann::json out;
                        out["query"] = q;
                        out["results"] = arr;
                        out["count"] = results.size();
                        sendJson(res, out, 200);
                    }
                    catch (const SkillBridgeException& e) {
                        sendError(res, 500, e.what());
                    }
                    catch (const std::exception& e) {
                        sendError(res, 500,
                            std::string("unexpected error: ") + e.what());
                    }
                });

            impl_->server.Get("/api/gigs/autocomplete",
                [this](const httplib::Request& req, httplib::Response& res) {
                    int userID = requireAuth(req, res);
                    if (userID < 0) return;

                    try {
                        std::string q;
                        if (req.has_param("q")) q = req.get_param_value("q");
                        // Trim
                        size_t start = q.find_first_not_of(" \t\r\n");
                        size_t end = q.find_last_not_of(" \t\r\n");
                        if (start != std::string::npos) {
                            q = q.substr(start, end - start + 1);
                        }
                        else {
                            q = "";
                        }

                        nlohmann::json arr = nlohmann::json::array();

                        if (q.size() >= 2) {
                            DataList<std::string> suggestions =
                                ctx_.gigManager->autocompleteSuggestions(q, 5);
                            for (int i = 0; i < suggestions.size(); ++i) {
                                arr.push_back(suggestions.get(i));
                            }
                        }

                        nlohmann::json out;
                        out["suggestions"] = arr;
                        sendJson(res, out, 200);
                    }
                    catch (const SkillBridgeException& e) {
                        sendError(res, 500, e.what());
                    }
                    catch (const std::exception& e) {
                        sendError(res, 500,
                            std::string("unexpected error: ") + e.what());
                    }
                });

            impl_->server.Get(R"(/api/gigs/(\d+))",
                [this](const httplib::Request& req, httplib::Response& res) {
                    int userID = requireAuth(req, res);
                    if (userID < 0) return;

                    try {
                        int gigID = 0;
                        if (!parsePositiveInt(req.matches[1], gigID)) {
                            sendError(res, 400, "invalid gig id");
                            return;
                        }

                        Gig g;
                        try {
                            g = ctx_.gigManager->findGigByID(gigID);
                        }
                        catch (const SkillBridgeException&) {
                            sendError(res, 404, "gig not found");
                            return;
                        }

                        std::unique_ptr<User> ownerUser(
                            ctx_.userManager->findUserByID(g.getOwnerID()));

                        if (!ownerUser) {
                            sendError(res, 500, "gig owner not found");
                            return;
                        }

                        Freelancer* freelancer =
                            dynamic_cast<Freelancer*>(ownerUser.get());
                        if (!freelancer) {
                            sendError(res, 500,
                                "gig owner is not a freelancer");
                            return;
                        }

                        int reviewCount = 0;
                        try {
                            DataList<Review> reviews =
                                ctx_.reviewManager->getReviewsForFreelancer(
                                    userID, g.getOwnerID());
                            reviewCount = reviews.size();
                        }
                        catch (...) {
                            reviewCount = 0;
                        }

                        std::unique_ptr<User> viewer(
                            ctx_.userManager->findUserByID(userID));

                        bool isOwner = false;
                        bool canOrder = false;
                        bool canEdit = false;
                        bool canDelete = false;

                        if (viewer) {
                            UserRole vRole = viewer->getRole();
                            isOwner = (viewer->getUserID() == g.getOwnerID());

                            canOrder = (vRole == UserRole::CLIENT)
                                && !isOwner
                                && g.getIsActive();

                            canEdit = (vRole == UserRole::FREELANCER)
                                && isOwner;

                            canDelete = (vRole == UserRole::ADMIN);
                        }

                        nlohmann::json out;
                        out["gig"] = sb::api::toGigDetailJson(g);
                        out["freelancer"] =
                            sb::api::toFreelancerDetailJson(*freelancer,
                                reviewCount);

                        nlohmann::json v;
                        v["isOwner"] = isOwner;
                        v["canOrder"] = canOrder;
                        v["canEdit"] = canEdit;
                        v["canDelete"] = canDelete;
                        out["viewer"] = v;

                        sendJson(res, out, 200);
                    }
                    catch (const SkillBridgeException& e) {
                        sendError(res, 500, e.what());
                    }
                    catch (const std::exception& e) {
                        sendError(res, 500,
                            std::string("unexpected error: ") + e.what());
                    }
                });

            impl_->server.Delete(R"(/api/gigs/(\d+))",
                [this](const httplib::Request& req, httplib::Response& res) {
                    int userID = requireAuth(req, res);
                    if (userID < 0) return;

                    try {
                        int gigID = 0;
                        if (!parsePositiveInt(req.matches[1], gigID)) {
                            sendError(res, 400, "invalid gig id");
                            return;
                        }

                        std::lock_guard<std::mutex> dbLock(
                            DatabaseManager::getInstance().getWriteMutex());

                        ctx_.gigManager->deleteGig(userID, gigID);

                        nlohmann::json out;
                        out["ok"] = true;
                        sendJson(res, out, 200);
                    }
                    catch (const UnauthorizedException& e) {
                        sendError(res, 403, e.what());
                    }
                    catch (const ValidationException& e) {
                        sendError(res, 400, e.what());
                    }
                    catch (const SkillBridgeException& e) {
                        sendError(res, 500, e.what());
                    }
                    catch (const std::exception& e) {
                        sendError(res, 500,
                            std::string("unexpected error: ") + e.what());
                    }
                });

            impl_->server.Post("/api/gigs",
                [this](const httplib::Request& req, httplib::Response& res) {
                    int userID = requireAuth(req, res);
                    if (userID < 0) return;

                    try {
                        auto body = nlohmann::json::parse(req.body);

                        std::string title = body.value("title", "");
                        std::string desc = body.value("description", "");
                        std::string catStr = body.value("category", "");

                        double price = 0.0;
                        if (body.contains("price") && body["price"].is_number()) {
                            price = body["price"].get<double>();
                        }

                        GigCategory category;
                        try {
                            category = gigCategoryFromString(catStr);
                        }
                        catch (const std::invalid_argument& e) {
                            sendError(res, 400, e.what());
                            return;
                        }

                        std::lock_guard<std::mutex> dbLock(
                            DatabaseManager::getInstance().getWriteMutex());

                        Gig newGig = ctx_.gigManager->createGig(
                            userID, title, desc, price, category);

                        nlohmann::json out;
                        out["gig"] = sb::api::toGigDetailJson(newGig);
                        sendJson(res, out, 201);
                    }
                    catch (const ValidationException& e) {
                        sendError(res, 400, e.what());
                    }
                    catch (const UnauthorizedException& e) {
                        sendError(res, 403, e.what());
                    }
                    catch (const SkillBridgeException& e) {
                        sendError(res, 500, e.what());
                    }
                    catch (const nlohmann::json::exception&) {
                        sendError(res, 400, "invalid JSON in request body");
                    }
                    catch (const std::exception& e) {
                        sendError(res, 500,
                            std::string("unexpected error: ") + e.what());
                    }
                });
            impl_->server.Put(R"(/api/gigs/(\d+))",
                [this](const httplib::Request& req, httplib::Response& res) {
                    int userID = requireAuth(req, res);
                    if (userID < 0) return;

                    try {
                        int gigID = 0;
                        if (!parsePositiveInt(req.matches[1], gigID)) {
                            sendError(res, 400, "invalid gig id");
                            return;
                        }

                        auto body = nlohmann::json::parse(req.body);

                        std::string title = body.value("title", "");
                        std::string desc = body.value("description", "");
                        std::string catStr = body.value("category", "");

                        double price = 0.0;
                        if (body.contains("price") && body["price"].is_number()) {
                            price = body["price"].get<double>();
                        }

                        GigCategory category;
                        try {
                            category = gigCategoryFromString(catStr);
                        }
                        catch (const std::invalid_argument& e) {
                            sendError(res, 400, e.what());
                            return;
                        }

                        std::lock_guard<std::mutex> dbLock(
                            DatabaseManager::getInstance().getWriteMutex());

                        ctx_.gigManager->updateGig(
                            userID, gigID, title, desc, price, category);
                        Gig updated = ctx_.gigManager->findGigByID(gigID);

                        nlohmann::json out;
                        out["gig"] = sb::api::toGigDetailJson(updated);
                        sendJson(res, out, 200);
                    }
                    catch (const ValidationException& e) {
                        sendError(res, 400, e.what());
                    }
                    catch (const UnauthorizedException& e) {
                        sendError(res, 403, e.what());
                    }
                    catch (const GigNotFoundException& e) {
                        sendError(res, 404, e.what());
                    }
                    catch (const SkillBridgeException& e) {
                        sendError(res, 500, e.what());
                    }
                    catch (const nlohmann::json::exception&) {
                        sendError(res, 400, "invalid JSON in request body");
                    }
                    catch (const std::exception& e) {
                        sendError(res, 500,
                            std::string("unexpected error: ") + e.what());
                    }
                });

            impl_->server.Patch(R"(/api/gigs/(\d+)/active)",
                [this](const httplib::Request& req, httplib::Response& res) {
                    int userID = requireAuth(req, res);
                    if (userID < 0) return;

                    try {
                        int gigID = 0;
                        if (!parsePositiveInt(req.matches[1], gigID)) {
                            sendError(res, 400, "invalid gig id");
                            return;
                        }

                        auto body = nlohmann::json::parse(req.body);
                        if (!body.contains("isActive") ||
                            !body["isActive"].is_boolean()) {
                            sendError(res, 400,
                                "isActive (boolean) is required");
                            return;
                        }
                        bool active = body["isActive"].get<bool>();

                        std::lock_guard<std::mutex> dbLock(
                            DatabaseManager::getInstance().getWriteMutex());

                        ctx_.gigManager->setGigActive(userID, gigID, active);

                        nlohmann::json gigJ;
                        gigJ["gigID"] = gigID;
                        gigJ["isActive"] = active;
                        nlohmann::json out;
                        out["gig"] = gigJ;
                        sendJson(res, out, 200);
                    }
                    catch (const UnauthorizedException& e) {
                        sendError(res, 403, e.what());
                    }
                    catch (const GigNotFoundException& e) {
                        sendError(res, 404, e.what());
                    }
                    catch (const ValidationException& e) {
                        sendError(res, 400, e.what());
                    }
                    catch (const SkillBridgeException& e) {
                        sendError(res, 500, e.what());
                    }
                    catch (const nlohmann::json::exception&) {
                        sendError(res, 400, "invalid JSON in request body");
                    }
                    catch (const std::exception& e) {
                        sendError(res, 500,
                            std::string("unexpected error: ") + e.what());
                    }
                });
        }

        void HttpServer::registerReviewRoutes() {

            impl_->server.Get(R"(/api/users/(\d+)/reviews)",
                [this](const httplib::Request& req, httplib::Response& res) {
                    int viewerID = requireAuth(req, res);
                    if (viewerID < 0) return;

                    try {
                        int targetID = 0;
                        if (!parsePositiveInt(req.matches[1], targetID)) {
                            sendError(res, 400, "invalid user id");
                            return;
                        }

                        int limit = 3;
                        if (req.has_param("limit")) {
                            try {
                                limit = std::stoi(req.get_param_value("limit"));
                            }
                            catch (...) {
                                limit = 3;
                            }
                        }
                        if (limit < 1) limit = 1;
                        if (limit > 50) limit = 50;
                        std::unique_ptr<User> target(
                            ctx_.userManager->findUserByID(targetID));
                        if (!target) {
                            sendError(res, 404, "user not found");
                            return;
                        }
                        if (target->getRole() != UserRole::FREELANCER) {
                            sendError(res, 400,
                                "user is not a freelancer");
                            return;
                        }

                        DataList<Review> all =
                            ctx_.reviewManager->getReviewsForFreelancer(
                                viewerID, targetID);

                        int total = all.size();
                        int returned = (total < limit) ? total : limit;

                        nlohmann::json arr = nlohmann::json::array();
                        for (int i = 0; i < returned; ++i) {
                            const Review& r = all.get(i);

                            std::string reviewerName = "Unknown";
                            std::unique_ptr<User> reviewer(
                                ctx_.userManager->findUserByID(
                                    r.getReviewerID()));
                            if (reviewer) {
                                reviewerName = reviewer->getName();
                            }

                            arr.push_back(
                                sb::api::toReviewCardJson(r, reviewerName));
                        }

                        nlohmann::json out;
                        out["reviews"] = arr;
                        out["totalCount"] = total;
                        sendJson(res, out, 200);
                    }
                    catch (const UnauthorizedException& e) {
                        sendError(res, 403, e.what());
                    }
                    catch (const SkillBridgeException& e) {
                        sendError(res, 500, e.what());
                    }
                    catch (const std::exception& e) {
                        sendError(res, 500,
                            std::string("unexpected error: ") + e.what());
                    }
                });
        }

        void HttpServer::registerUserRoutes() {

            // ----- GET /api/users/:id  --------------------------------------
            impl_->server.Get(R"(/api/users/(\d+))",
                [this](const httplib::Request& req, httplib::Response& res) {
                    int viewerID = requireAuth(req, res);
                    if (viewerID < 0) return;

                    try {
                        int targetID = 0;
                        if (!parsePositiveInt(req.matches[1], targetID)) {
                            sendError(res, 400, "invalid user id");
                            return;
                        }

                        std::unique_ptr<User> target(
                            ctx_.userManager->findUserByID(targetID));
                        if (!target) {
                            sendError(res, 404, "user not found");
                            return;
                        }

                        nlohmann::json gigsArr = nlohmann::json::array();
                        nlohmann::json reviewsArr = nlohmann::json::array();
                        int reviewCount = 0;

                        if (target->getRole() == UserRole::FREELANCER) {
                            try {
                                DataList<Review> reviews =
                                    ctx_.reviewManager->getReviewsForFreelancer(
                                        viewerID, targetID);
                                reviewCount = reviews.size();
                                for (int i = 0; i < reviews.size(); ++i) {
                                    const Review& r = reviews.get(i);
                                    std::string reviewerName = "Unknown";
                                    std::unique_ptr<User> reviewer(
                                        ctx_.userManager->findUserByID(
                                            r.getReviewerID()));
                                    if (reviewer) {
                                        reviewerName = reviewer->getName();
                                    }
                                    reviewsArr.push_back(
                                        sb::api::toReviewCardJson(
                                            r, reviewerName));
                                }
                            }
                            catch (...) {
                                reviewCount = 0;
                            }

                            try {
                                DataList<Gig> ownedGigs =
                                    ctx_.gigManager->findGigsByOwner(targetID);
                                for (int i = 0; i < ownedGigs.size(); ++i) {
                                    const Gig& g = ownedGigs.get(i);
                                    if (!g.getIsActive()) continue;

                                    std::string fName;
                                    double fRating;
                                    lookupFreelancerForCard(
                                        ctx_.userManager, g.getOwnerID(),
                                        fName, fRating);

                                    gigsArr.push_back(
                                        sb::api::toBrowseCardJson(
                                            g, fName, fRating));
                                }
                            }
                            catch (...) {
                            }
                        }

                        nlohmann::json out;
                        out["user"] = sb::api::toUserPublicProfileJson(
                            *target, reviewCount);
                        out["gigs"] = gigsArr;
                        out["reviews"] = reviewsArr;
                        sendJson(res, out, 200);
                    }
                    catch (const SkillBridgeException& e) {
                        sendError(res, 500, e.what());
                    }
                    catch (const std::exception& e) {
                        sendError(res, 500,
                            std::string("unexpected error: ") + e.what());
                    }
                });

            // ----- PUT /api/me  ----------------------------------------------
            impl_->server.Put("/api/me",
                [this](const httplib::Request& req, httplib::Response& res) {
                    int userID = requireAuth(req, res);
                    if (userID < 0) return;

                    try {
                        auto body = nlohmann::json::parse(req.body);

                        std::string newName = body.value("name", "");
                        bool hasPortfolio = body.contains("portfolio")
                            && body["portfolio"].is_string();
                        bool hasSkills = body.contains("skills")
                            && body["skills"].is_string();

                        std::lock_guard<std::mutex> dbLock(
                            DatabaseManager::getInstance().getWriteMutex());

                        if (!newName.empty()) {
                            ctx_.userManager->updateProfileForUser(
                                userID, newName);
                        }

                        if (hasPortfolio || hasSkills) {
                            std::string portfolio = hasPortfolio
                                ? body["portfolio"].get<std::string>()
                                : "";
                            std::string skills = hasSkills
                                ? body["skills"].get<std::string>()
                                : "";
                            if (!hasPortfolio || !hasSkills) {
                                std::unique_ptr<User> existing(
                                    ctx_.userManager->findUserByID(userID));
                                if (existing) {
                                    auto* f = dynamic_cast<Freelancer*>(
                                        existing.get());
                                    if (f) {
                                        if (!hasPortfolio) portfolio = f->getPortfolio();
                                        if (!hasSkills) skills = f->getSkills();
                                    }
                                }
                            }
                            ctx_.userManager->updateFreelancerFields(
                                userID, portfolio, skills);
                        }

                        std::unique_ptr<User> updated(
                            ctx_.userManager->findUserByID(userID));
                        if (!updated) {
                            sendError(res, 500, "could not reload user");
                            return;
                        }
                        nlohmann::json out;
                        out["user"] = sb::api::toJson(*updated);
                        sendJson(res, out, 200);
                    }
                    catch (const ValidationException& e) {
                        sendError(res, 400, e.what());
                    }
                    catch (const UnauthorizedException& e) {
                        sendError(res, 403, e.what());
                    }
                    catch (const AuthenticationException& e) {
                        sendError(res, 401, e.what());
                    }
                    catch (const SkillBridgeException& e) {
                        sendError(res, 500, e.what());
                    }
                    catch (const nlohmann::json::exception&) {
                        sendError(res, 400, "invalid JSON in request body");
                    }
                    catch (const std::exception& e) {
                        sendError(res, 500,
                            std::string("unexpected error: ") + e.what());
                    }
                });

            impl_->server.Post("/api/me/password",
                [this](const httplib::Request& req, httplib::Response& res) {
                    int userID = requireAuth(req, res);
                    if (userID < 0) return;

                    try {
                        auto body = nlohmann::json::parse(req.body);
                        std::string oldPwd = body.value("oldPassword", "");
                        std::string newPwd = body.value("newPassword", "");

                        if (oldPwd.empty() || newPwd.empty()) {
                            sendError(res, 400,
                                "oldPassword and newPassword are required");
                            return;
                        }

                        std::lock_guard<std::mutex> dbLock(
                            DatabaseManager::getInstance().getWriteMutex());

                        ctx_.userManager->changePasswordForUser(
                            userID, oldPwd, newPwd);

                        nlohmann::json out;
                        out["ok"] = true;
                        sendJson(res, out, 200);
                    }
                    catch (const ValidationException& e) {
                        sendError(res, 400, e.what());
                    }
                    catch (const AuthenticationException& e) {
                        sendError(res, 401, e.what());
                    }
                    catch (const SkillBridgeException& e) {
                        sendError(res, 500, e.what());
                    }
                    catch (const nlohmann::json::exception&) {
                        sendError(res, 400, "invalid JSON in request body");
                    }
                    catch (const std::exception& e) {
                        sendError(res, 500,
                            std::string("unexpected error: ") + e.what());
                    }
                });

            impl_->server.Delete("/api/me",
                [this](const httplib::Request& req, httplib::Response& res) {
                    int userID = requireAuth(req, res);
                    if (userID < 0) return;

                    try {
                        auto body = nlohmann::json::parse(req.body);
                        std::string password = body.value("password", "");

                        if (password.empty()) {
                            sendError(res, 400, "password is required");
                            return;
                        }

                        std::lock_guard<std::mutex> dbLock(
                            DatabaseManager::getInstance().getWriteMutex());

                        ctx_.userManager->deleteAccountForUser(
                            userID, password);

                        {
                            std::lock_guard<std::mutex> sessLock(impl_->sessionMutex);
                            for (auto it = impl_->sessions.begin();
                                it != impl_->sessions.end();) {
                                if (it->second == userID) {
                                    it = impl_->sessions.erase(it);
                                }
                                else {
                                    ++it;
                                }
                            }
                        }

                        nlohmann::json out;
                        out["ok"] = true;
                        sendJson(res, out, 200);
                    }
                    catch (const ValidationException& e) {
                        sendError(res, 400, e.what());
                    }
                    catch (const AuthenticationException& e) {
                        sendError(res, 401, e.what());
                    }
                    catch (const SkillBridgeException& e) {
                        sendError(res, 500, e.what());
                    }
                    catch (const nlohmann::json::exception&) {
                        sendError(res, 400, "invalid JSON in request body");
                    }
                    catch (const std::exception& e) {
                        sendError(res, 500,
                            std::string("unexpected error: ") + e.what());
                    }
                });
        }

        void HttpServer::registerOrderRoutes() {

            // ----- GET /api/orders -----------------------------------------------
            impl_->server.Get("/api/orders",
                [this](const httplib::Request& req, httplib::Response& res) {
                    int userID = requireAuth(req, res);
                    if (userID < 0) return;

                    try {
                        std::unique_ptr<User> viewer(
                            ctx_.userManager->findUserByID(userID));
                        if (!viewer) {
                            sendError(res, 401, "user not found");
                            return;
                        }

                        std::string roleParam = "buyer";
                        if (req.has_param("role"))
                            roleParam = req.get_param_value("role");

                        DataList<Order> orders;

                        if (roleParam == "all") {
                            if (viewer->getRole() != UserRole::ADMIN) {
                                sendError(res, 403, "admin only");
                                return;
                            }
                            orders = ctx_.orderManager->findAll(userID);
                        }
                        else if (roleParam == "seller") {
                            orders = ctx_.orderManager->findBySeller(userID);
                        }
                        else {
                            orders = ctx_.orderManager->findByBuyer(userID);
                        }

                        nlohmann::json arr = nlohmann::json::array();
                        for (int i = 0; i < orders.size(); ++i) {
                            const Order& o = orders.get(i);

                            // Enrich gig title
                            std::string gigTitle = "Unknown gig";
                            try {
                                Gig g = ctx_.gigManager->findGigByID(o.getGigID());
                                gigTitle = g.getTitle();
                            }
                            catch (...) {}

                            // Determine other party based on viewer perspective
                            int otherID;
                            if (roleParam == "seller") {
                                otherID = o.getBuyerID();
                            }
                            else if (roleParam == "buyer") {
                                otherID = o.getSellerID();
                            }
                            else {
                                // Admin: show seller as other party
                                otherID = o.getSellerID();
                            }

                            std::string otherName = "Unknown";
                            std::unique_ptr<User> other(
                                ctx_.userManager->findUserByID(otherID));
                            if (other) otherName = other->getName();

                            arr.push_back(sb::api::toOrderCardJson(
                                o, gigTitle, otherName, otherID));
                        }

                        nlohmann::json out;
                        out["orders"] = arr;
                        out["count"] = orders.size();
                        sendJson(res, out, 200);
                    }
                    catch (const UnauthorizedException& e) {
                        sendError(res, 403, e.what());
                    }
                    catch (const SkillBridgeException& e) {
                        sendError(res, 500, e.what());
                    }
                    catch (const std::exception& e) {
                        sendError(res, 500,
                            std::string("unexpected error: ") + e.what());
                    }
                });

            // ----- GET /api/orders/:id -------------------------------------------
            impl_->server.Get(R"(/api/orders/(\d+))",
                [this](const httplib::Request& req, httplib::Response& res) {
                    int userID = requireAuth(req, res);
                    if (userID < 0) return;

                    try {
                        int orderID = 0;
                        if (!parsePositiveInt(req.matches[1], orderID)) {
                            sendError(res, 400, "invalid order id");
                            return;
                        }

                        Order o;
                        try {
                            o = ctx_.orderManager->findOrderByID(userID, orderID);
                        }
                        catch (const OrderNotFoundException&) {
                            sendError(res, 404, "order not found");
                            return;
                        }

                        std::string gigTitle = "Unknown gig";
                        try {
                            Gig g = ctx_.gigManager->findGigByID(o.getGigID());
                            gigTitle = g.getTitle();
                        }
                        catch (...) {}

                        bool isBuyer = (userID == o.getBuyerID());
                        bool isSeller = (userID == o.getSellerID());
                        std::unique_ptr<User> viewer(
                            ctx_.userManager->findUserByID(userID));
                        bool isAdmin = viewer &&
                            (viewer->getRole() == UserRole::ADMIN);

                        int otherID = isBuyer ? o.getSellerID() : o.getBuyerID();
                        std::string otherName = "Unknown";
                        std::unique_ptr<User> other(
                            ctx_.userManager->findUserByID(otherID));
                        if (other) otherName = other->getName();

                        OrderStatus st = o.getStatus();
                        bool canCancel = (st == OrderStatus::PENDING ||
                            st == OrderStatus::IN_PROGRESS) &&
                            (isBuyer || isSeller || isAdmin);
                        bool canAccept = (st == OrderStatus::PENDING) &&
                            (isSeller || isAdmin);
                        bool canComplete = (st == OrderStatus::IN_PROGRESS) &&
                            (isSeller || isAdmin);

                        bool canReview = false;
                        if (isBuyer && st == OrderStatus::COMPLETED) {
                            try {
                                DataList<Review> existing =
                                    ctx_.reviewManager->getReviewsForFreelancer(
                                        userID, o.getSellerID());
                                bool alreadyReviewed = false;
                                for (int i = 0; i < existing.size(); ++i) {
                                    if (existing.get(i).getOrderID() == o.getOrderID()) {
                                        alreadyReviewed = true;
                                        break;
                                    }
                                }
                                canReview = !alreadyReviewed;
                            }
                            catch (...) {
                                canReview = false;
                            }
                        }

                        nlohmann::json out;
                        out["order"] = sb::api::toOrderCardJson(
                            o, gigTitle, otherName, otherID);

                        nlohmann::json v;
                        v["isBuyer"] = isBuyer;
                        v["isSeller"] = isSeller;
                        v["isAdmin"] = isAdmin;
                        v["canCancel"] = canCancel;
                        v["canAccept"] = canAccept;
                        v["canComplete"] = canComplete;
                        v["canReview"] = canReview;
                        out["viewer"] = v;

                        sendJson(res, out, 200);
                    }
                    catch (const UnauthorizedException& e) {
                        sendError(res, 403, e.what());
                    }
                    catch (const SkillBridgeException& e) {
                        sendError(res, 500, e.what());
                    }
                    catch (const std::exception& e) {
                        sendError(res, 500,
                            std::string("unexpected error: ") + e.what());
                    }
                });

            // ----- POST /api/orders ----------------------------------------------
            impl_->server.Post("/api/orders",
                [this](const httplib::Request& req, httplib::Response& res) {
                    int userID = requireAuth(req, res);
                    if (userID < 0) return;

                    try {
                        auto body = nlohmann::json::parse(req.body);
                        int gigID = 0;
                        if (body.contains("gigID") && body["gigID"].is_number_integer()) {
                            gigID = body["gigID"].get<int>();
                        }
                        std::string deadline = body.value("deadline", "");

                        if (gigID <= 0) {
                            sendError(res, 400, "gigID is required");
                            return;
                        }
                        if (deadline.empty()) {
                            sendError(res, 400, "deadline is required");
                            return;
                        }

                        std::lock_guard<std::mutex> dbLock(
                            DatabaseManager::getInstance().getWriteMutex());

                        Order o = ctx_.orderManager->placeOrder(
                            userID, gigID, deadline);

                        std::string gigTitle = "Unknown gig";
                        try {
                            Gig g = ctx_.gigManager->findGigByID(o.getGigID());
                            gigTitle = g.getTitle();
                        }
                        catch (...) {}

                        std::string otherName = "Unknown";
                        int otherID = o.getSellerID();
                        std::unique_ptr<User> other(
                            ctx_.userManager->findUserByID(otherID));
                        if (other) otherName = other->getName();

                        std::unique_ptr<User> updated(
                            ctx_.userManager->findUserByID(userID));
                        nlohmann::json out;
                        out["order"] = sb::api::toOrderCardJson(
                            o, gigTitle, otherName, otherID);
                        if (updated) {
                            out["updatedBalance"] = updated->getBalance();
                        }
                        sendJson(res, out, 201);
                    }
                    catch (const ValidationException& e) {
                        sendError(res, 400, e.what());
                    }
                    catch (const UnauthorizedException& e) {
                        sendError(res, 403, e.what());
                    }
                    catch (const InsufficientFundsException& e) {
                        sendError(res, 400, e.what());
                    }
                    catch (const GigNotFoundException& e) {
                        sendError(res, 404, e.what());
                    }
                    catch (const SkillBridgeException& e) {
                        sendError(res, 500, e.what());
                    }
                    catch (const nlohmann::json::exception&) {
                        sendError(res, 400, "invalid JSON in request body");
                    }
                    catch (const std::exception& e) {
                        sendError(res, 500,
                            std::string("unexpected error: ") + e.what());
                    }
                });

            // ----- PATCH /api/orders/:id/status ----------------------------------
            impl_->server.Patch(R"(/api/orders/(\d+)/status)",
                [this](const httplib::Request& req, httplib::Response& res) {
                    int userID = requireAuth(req, res);
                    if (userID < 0) return;

                    try {
                        int orderID = 0;
                        if (!parsePositiveInt(req.matches[1], orderID)) {
                            sendError(res, 400, "invalid order id");
                            return;
                        }

                        auto body = nlohmann::json::parse(req.body);
                        std::string statusStr = body.value("status", "");
                        if (statusStr.empty()) {
                            sendError(res, 400, "status is required");
                            return;
                        }

                        OrderStatus newStatus;
                        try {
                            newStatus = orderStatusFromString(statusStr);
                        }
                        catch (const ValidationException&) {
                            sendError(res, 400, "invalid status value");
                            return;
                        }

                        std::lock_guard<std::mutex> dbLock(
                            DatabaseManager::getInstance().getWriteMutex());

                        if (newStatus == OrderStatus::CANCELLED) {
                            ctx_.orderManager->cancelOrder(userID, orderID);
                        }
                        else {
                            ctx_.orderManager->updateStatus(
                                userID, orderID, newStatus);
                        }

                        Order updated = ctx_.orderManager->findOrderByID(
                            userID, orderID);

                        std::string gigTitle = "Unknown gig";
                        try {
                            Gig g = ctx_.gigManager->findGigByID(
                                updated.getGigID());
                            gigTitle = g.getTitle();
                        }
                        catch (...) {}

                        bool isBuyer = (userID == updated.getBuyerID());
                        int otherID = isBuyer
                            ? updated.getSellerID()
                            : updated.getBuyerID();
                        std::string otherName = "Unknown";
                        std::unique_ptr<User> other(
                            ctx_.userManager->findUserByID(otherID));
                        if (other) otherName = other->getName();

                        std::unique_ptr<User> viewer(
                            ctx_.userManager->findUserByID(userID));

                        nlohmann::json out;
                        out["order"] = sb::api::toOrderCardJson(
                            updated, gigTitle, otherName, otherID);
                        if (viewer) {
                            out["updatedBalance"] = viewer->getBalance();
                        }
                        sendJson(res, out, 200);
                    }
                    catch (const ValidationException& e) {
                        sendError(res, 400, e.what());
                    }
                    catch (const UnauthorizedException& e) {
                        sendError(res, 403, e.what());
                    }
                    catch (const OrderNotFoundException& e) {
                        sendError(res, 404, e.what());
                    }
                    catch (const InsufficientFundsException& e) {
                        sendError(res, 400, e.what());
                    }
                    catch (const SkillBridgeException& e) {
                        sendError(res, 500, e.what());
                    }
                    catch (const nlohmann::json::exception&) {
                        sendError(res, 400, "invalid JSON in request body");
                    }
                    catch (const std::exception& e) {
                        sendError(res, 500,
                            std::string("unexpected error: ") + e.what());
                    }
                });

            // ----- POST /api/orders/:id/review -----------------------------------
            impl_->server.Post(R"(/api/orders/(\d+)/review)",
                [this](const httplib::Request& req, httplib::Response& res) {
                    int userID = requireAuth(req, res);
                    if (userID < 0) return;

                    try {
                        int orderID = 0;
                        if (!parsePositiveInt(req.matches[1], orderID)) {
                            sendError(res, 400, "invalid order id");
                            return;
                        }

                        auto body = nlohmann::json::parse(req.body);

                        int rating = 0;
                        if (body.contains("rating") &&
                            body["rating"].is_number_integer()) {
                            rating = body["rating"].get<int>();
                        }
                        std::string comment = body.value("comment", "");

                        if (rating < 1 || rating > 5) {
                            sendError(res, 400,
                                "rating must be between 1 and 5");
                            return;
                        }
                        if (comment.empty()) {
                            sendError(res, 400, "comment is required");
                            return;
                        }

                        Review created;
                        {
                            std::lock_guard<std::mutex> dbLock(
                                DatabaseManager::getInstance().getWriteMutex());
                            created = ctx_.reviewManager->submitReview(
                                userID, orderID, rating, comment);
                        }

                        std::string reviewerName = "Unknown";
                        std::unique_ptr<User> reviewer(
                            ctx_.userManager->findUserByID(userID));
                        if (reviewer) reviewerName = reviewer->getName();

                        nlohmann::json out;
                        out["review"] = sb::api::toReviewCardJson(
                            created, reviewerName);
                        sendJson(res, out, 201);
                    }
                    catch (const ValidationException& e) {
                        sendError(res, 400, e.what());
                    }
                    catch (const UnauthorizedException& e) {
                        sendError(res, 403, e.what());
                    }
                    catch (const DuplicateEntryException& e) {
                        sendError(res, 409, e.what());
                    }
                    catch (const OrderNotFoundException& e) {
                        sendError(res, 404, e.what());
                    }
                    catch (const SkillBridgeException& e) {
                        sendError(res, 500, e.what());
                    }
                    catch (const nlohmann::json::exception&) {
                        sendError(res, 400, "invalid JSON in request body");
                    }
                    catch (const std::exception& e) {
                        sendError(res, 500,
                            std::string("unexpected error: ") + e.what());
                    }
                });

        }

        void HttpServer::registerMessageRoutes() {

            // ----- GET /api/messages/unread --------------------------------------
            impl_->server.Get("/api/messages/unread",
                [this](const httplib::Request& req, httplib::Response& res) {
                    int userID = requireAuth(req, res);
                    if (userID < 0) return;

                    try {
                        int count;
                        {
                            std::lock_guard<std::mutex> dbLock(
                                DatabaseManager::getInstance().getWriteMutex());
                            count = ctx_.messageManager->countUnread(userID, userID);
                        }

                        nlohmann::json out;
                        out["unreadCount"] = count;
                        sendJson(res, out, 200);
                    }
                    catch (const SkillBridgeException& e) {
                        sendError(res, 500, e.what());
                    }
                    catch (const std::exception& e) {
                        sendError(res, 500,
                            std::string("unexpected error: ") + e.what());
                    }
                });

            // ----- GET /api/messages/inbox ---------------------------------------
            impl_->server.Get("/api/messages/inbox",
                [this](const httplib::Request& req, httplib::Response& res) {
                    int userID = requireAuth(req, res);
                    if (userID < 0) return;

                    try {
                        int totalUnread;
                        DataList<int> partnerList;
                        {
                            std::lock_guard<std::mutex> dbLock(
                                DatabaseManager::getInstance().getWriteMutex());
                            totalUnread = ctx_.messageManager->countUnread(userID, userID);
                            partnerList = ctx_.messageManager->getConversationPartners(
                                userID, userID);
                        }
                        std::vector<int> partnerIDs;
                        for (int i = 0; i < partnerList.size(); ++i) {
                            partnerIDs.push_back(partnerList.get(i));
                        }

                        nlohmann::json convArr = nlohmann::json::array();

                        for (size_t p = 0; p < partnerIDs.size(); ++p) {
                            int otherID = partnerIDs[p];

                            DataList<DecodedMessage> conv;
                            {
                                std::lock_guard<std::mutex> dbLock(
                                    DatabaseManager::getInstance().getWriteMutex());
                                conv = ctx_.messageManager->getConversation(
                                    userID, userID, otherID);
                            }

                            if (conv.size() == 0) continue;

                            // Last message is the most recent (conversation
                            // is oldest-first, so last element).
                            const DecodedMessage& last = conv.get(conv.size() - 1);

                            // Count unread in this conversation (messages
                            // where I am receiver and isRead == false).
                            int unread = 0;
                            for (int i = 0; i < conv.size(); ++i) {
                                const DecodedMessage& m = conv.get(i);
                                if (m.receiverID == userID && !m.isRead) {
                                    ++unread;
                                }
                            }

                            // Lookup other party name
                            std::string otherName = "Unknown";
                            std::unique_ptr<User> other(
                                ctx_.userManager->findUserByID(otherID));
                            if (other) otherName = other->getName();

                            std::string preview = last.text;
                            if (preview.size() > 80) {
                                preview = preview.substr(0, 77) + "...";
                            }

                            nlohmann::json c;
                            c["otherPartyID"] = otherID;
                            c["otherPartyName"] = otherName;
                            c["lastMessage"] = preview;
                            c["lastTimestamp"] = last.timestamp;
                            c["unreadCount"] = unread;
                            c["isLastMine"] = (last.senderID == userID);
                            convArr.push_back(c);
                        }

                        std::sort(convArr.begin(), convArr.end(),
                            [](const nlohmann::json& a, const nlohmann::json& b) {
                                return a["lastTimestamp"].get<std::string>() >
                                    b["lastTimestamp"].get<std::string>();
                            });

                        nlohmann::json out;
                        out["conversations"] = convArr;
                        out["totalUnread"] = totalUnread;
                        sendJson(res, out, 200);
                    }
                    catch (const SkillBridgeException& e) {
                        sendError(res, 500, e.what());
                    }
                    catch (const std::exception& e) {
                        sendError(res, 500,
                            std::string("unexpected error: ") + e.what());
                    }
                });

            // ----- GET /api/messages/conversation/:otherUserID -------------------
            impl_->server.Get(R"(/api/messages/conversation/(\d+))",
                [this](const httplib::Request& req, httplib::Response& res) {
                    int userID = requireAuth(req, res);
                    if (userID < 0) return;

                    try {
                        int otherID = 0;
                        if (!parsePositiveInt(req.matches[1], otherID)) {
                            sendError(res, 400, "invalid user id");
                            return;
                        }

                        std::unique_ptr<User> otherUser(
                            ctx_.userManager->findUserByID(otherID));
                        if (!otherUser) {
                            sendError(res, 404, "user not found");
                            return;
                        }

                        DataList<DecodedMessage> conv;
                        {
                            std::lock_guard<std::mutex> dbLock(
                                DatabaseManager::getInstance().getWriteMutex());

                            ctx_.messageManager->markConversationRead(
                                userID, userID, otherID);

                            conv = ctx_.messageManager->getConversation(
                                userID, userID, otherID);
                        }
                        std::string myName = "You";
                        {
                            std::unique_ptr<User> me(
                                ctx_.userManager->findUserByID(userID));
                            if (me) myName = me->getName();
                        }
                        std::string otherName = otherUser->getName();

                        nlohmann::json arr = nlohmann::json::array();
                        for (int i = 0; i < conv.size(); ++i) {
                            const DecodedMessage& dm = conv.get(i);
                            std::string sName = (dm.senderID == userID)
                                ? myName : otherName;
                            arr.push_back(
                                sb::api::toMessageJson(dm, sName));
                        }

                        nlohmann::json otherParty;
                        otherParty["userID"] = otherID;
                        otherParty["name"] = otherName;

                        nlohmann::json out;
                        out["messages"] = arr;
                        out["otherParty"] = otherParty;
                        sendJson(res, out, 200);
                    }
                    catch (const UnauthorizedException& e) {
                        sendError(res, 403, e.what());
                    }
                    catch (const SkillBridgeException& e) {
                        sendError(res, 500, e.what());
                    }
                    catch (const std::exception& e) {
                        sendError(res, 500,
                            std::string("unexpected error: ") + e.what());
                    }
                });

            // ----- POST /api/messages --------------------------------------------
            impl_->server.Post("/api/messages",
                [this](const httplib::Request& req, httplib::Response& res) {
                    int userID = requireAuth(req, res);
                    if (userID < 0) return;

                    try {
                        auto body = nlohmann::json::parse(req.body);
                        int receiverID = 0;
                        if (body.contains("receiverID") &&
                            body["receiverID"].is_number_integer()) {
                            receiverID = body["receiverID"].get<int>();
                        }
                        std::string text = body.value("text", "");

                        if (receiverID <= 0) {
                            sendError(res, 400, "receiverID is required");
                            return;
                        }
                        if (text.empty()) {
                            sendError(res, 400, "text is required");
                            return;
                        }

                        Message saved;
                        {
                            std::lock_guard<std::mutex> dbLock(
                                DatabaseManager::getInstance().getWriteMutex());
                            saved = ctx_.messageManager->sendMessage(
                                userID, userID, receiverID, text);
                        }

                        // Build the DecodedMessage for the response
                        // (the caller just sent it, so we know the plaintext)
                        DecodedMessage dm;
                        dm.messageID = saved.getMessageID();
                        dm.senderID = saved.getSenderID();
                        dm.receiverID = saved.getReceiverID();
                        dm.text = text;
                        dm.timestamp = saved.getTimestamp();
                        dm.isRead = saved.getIsRead();

                        std::string senderName = "You";
                        {
                            std::unique_ptr<User> sender(
                                ctx_.userManager->findUserByID(userID));
                            if (sender) senderName = sender->getName();
                        }

                        nlohmann::json out;
                        out["message"] = sb::api::toMessageJson(dm, senderName);
                        sendJson(res, out, 201);
                    }
                    catch (const ValidationException& e) {
                        sendError(res, 400, e.what());
                    }
                    catch (const UnauthorizedException& e) {
                        sendError(res, 403, e.what());
                    }
                    catch (const SkillBridgeException& e) {
                        sendError(res, 500, e.what());
                    }
                    catch (const nlohmann::json::exception&) {
                        sendError(res, 400, "invalid JSON in request body");
                    }
                    catch (const std::exception& e) {
                        sendError(res, 500,
                            std::string("unexpected error: ") + e.what());
                    }
                });
        }

        void HttpServer::registerEndorsementRoutes() {

            // ----- POST /api/endorsements ----------------------------------------
            impl_->server.Post("/api/endorsements",
                [this](const httplib::Request& req, httplib::Response& res) {
                    int userID = requireAuth(req, res);
                    if (userID < 0) return;

                    try {
                        auto body = nlohmann::json::parse(req.body);

                        int targetUserID = 0;
                        if (body.contains("targetUserID") &&
                            body["targetUserID"].is_number_integer()) {
                            targetUserID = body["targetUserID"].get<int>();
                        }
                        std::string skill = body.value("skill", "");
                        double weight = 1.0;
                        if (body.contains("weight") && body["weight"].is_number()) {
                            weight = body["weight"].get<double>();
                        }

                        if (targetUserID <= 0) {
                            sendError(res, 400, "targetUserID is required");
                            return;
                        }
                        if (skill.empty()) {
                            sendError(res, 400, "skill is required");
                            return;
                        }

                        Endorsement created;
                        {
                            std::lock_guard<std::mutex> dbLock(
                                DatabaseManager::getInstance().getWriteMutex());
                            created = ctx_.skillGraphManager->endorseUser(
                                userID, targetUserID, skill, weight);
                        }

                        std::string targetName = "Unknown";
                        std::unique_ptr<User> target(
                            ctx_.userManager->findUserByID(targetUserID));
                        if (target) targetName = target->getName();

                        nlohmann::json out;
                        out["endorsement"] = sb::api::toEndorsementGivenJson(
                            created, targetName);
                        sendJson(res, out, 201);
                    }
                    catch (const ValidationException& e) {
                        sendError(res, 400, e.what());
                    }
                    catch (const DuplicateEntryException& e) {
                        sendError(res, 409, e.what());
                    }
                    catch (const UnauthorizedException& e) {
                        sendError(res, 403, e.what());
                    }
                    catch (const DatabaseException& e) {
                        // target user not found
                        sendError(res, 404, e.what());
                    }
                    catch (const SkillBridgeException& e) {
                        sendError(res, 500, e.what());
                    }
                    catch (const nlohmann::json::exception&) {
                        sendError(res, 400, "invalid JSON in request body");
                    }
                    catch (const std::exception& e) {
                        sendError(res, 500,
                            std::string("unexpected error: ") + e.what());
                    }
                });

            // ----- DELETE /api/endorsements/:id ----------------------------------
            impl_->server.Delete(R"(/api/endorsements/(\d+))",
                [this](const httplib::Request& req, httplib::Response& res) {
                    int userID = requireAuth(req, res);
                    if (userID < 0) return;

                    try {
                        int endorsementID = 0;
                        if (!parsePositiveInt(req.matches[1], endorsementID)) {
                            sendError(res, 400, "invalid endorsement id");
                            return;
                        }

                        {
                            std::lock_guard<std::mutex> dbLock(
                                DatabaseManager::getInstance().getWriteMutex());
                            ctx_.skillGraphManager->removeEndorsement(
                                userID, endorsementID);
                        }

                        nlohmann::json out;
                        out["ok"] = true;
                        sendJson(res, out, 200);
                    }
                    catch (const UnauthorizedException& e) {
                        sendError(res, 403, e.what());
                    }
                    catch (const SkillBridgeException& e) {
                        sendError(res, 404, e.what());
                    }
                    catch (const std::exception& e) {
                        sendError(res, 500,
                            std::string("unexpected error: ") + e.what());
                    }
                });

            impl_->server.Get(R"(/api/users/(\d+)/endorsements)",
                [this](const httplib::Request& req, httplib::Response& res) {
                    int viewerID = requireAuth(req, res);
                    if (viewerID < 0) return;

                    try {
                        int targetID = 0;
                        if (!parsePositiveInt(req.matches[1], targetID)) {
                            sendError(res, 400, "invalid user id");
                            return;
                        }

                        std::unique_ptr<User> target(
                            ctx_.userManager->findUserByID(targetID));
                        if (!target) {
                            sendError(res, 404, "user not found");
                            return;
                        }

                        DataList<Endorsement> endorsements;
                        {
                            std::lock_guard<std::mutex> dbLock(
                                DatabaseManager::getInstance().getWriteMutex());
                            endorsements = ctx_.skillGraphManager->getEndorsementsFor(
                                viewerID, targetID);
                        }

                        nlohmann::json arr = nlohmann::json::array();
                        for (int i = 0; i < endorsements.size(); ++i) {
                            const Endorsement& e = endorsements.get(i);
                            std::string fromName = "Unknown";
                            std::unique_ptr<User> from(
                                ctx_.userManager->findUserByID(e.getFromUserID()));
                            if (from) fromName = from->getName();

                            arr.push_back(
                                sb::api::toEndorsementReceivedJson(e, fromName));
                        }

                        nlohmann::json out;
                        out["endorsements"] = arr;
                        out["count"] = endorsements.size();
                        sendJson(res, out, 200);
                    }
                    catch (const UnauthorizedException& e) {
                        sendError(res, 403, e.what());
                    }
                    catch (const SkillBridgeException& e) {
                        sendError(res, 500, e.what());
                    }
                    catch (const std::exception& e) {
                        sendError(res, 500,
                            std::string("unexpected error: ") + e.what());
                    }
                });

            impl_->server.Get("/api/endorsements/given",
                [this](const httplib::Request& req, httplib::Response& res) {
                    int userID = requireAuth(req, res);
                    if (userID < 0) return;

                    try {
                        DataList<Endorsement> given;
                        {
                            std::lock_guard<std::mutex> dbLock(
                                DatabaseManager::getInstance().getWriteMutex());
                        }

                        given = ctx_.skillGraphManager->getEndorsementsBy(
                            userID, userID);

                        nlohmann::json arr = nlohmann::json::array();
                        for (int i = 0; i < given.size(); ++i) {
                            const Endorsement& e = given.get(i);
                            std::string toName = "Unknown";
                            std::unique_ptr<User> to(
                                ctx_.userManager->findUserByID(e.getToUserID()));
                            if (to) toName = to->getName();

                            arr.push_back(
                                sb::api::toEndorsementGivenJson(e, toName));
                        }

                        nlohmann::json out;
                        out["endorsements"] = arr;
                        out["count"] = static_cast<int>(arr.size());
                        sendJson(res, out, 200);
                    }
                    catch (const UnauthorizedException& e) {
                        sendError(res, 403, e.what());
                    }
                    catch (const SkillBridgeException& e) {
                        sendError(res, 500, e.what());
                    }
                    catch (const std::exception& e) {
                        sendError(res, 500,
                            std::string("unexpected error: ") + e.what());
                    }
                });

            // ----- GET /api/endorsements/rankings --------------------------------
            impl_->server.Get("/api/endorsements/rankings",
                [this](const httplib::Request& req, httplib::Response& res) {
                    int userID = requireAuth(req, res);
                    if (userID < 0) return;

                    try {
                        int limit = 10;
                        if (req.has_param("limit")) {
                            try {
                                limit = std::stoi(req.get_param_value("limit"));
                            }
                            catch (...) {
                                limit = 10;
                            }
                        }
                        if (limit < 1) limit = 1;
                        if (limit > 50) limit = 50;

                        DataList<RankedUser> ranked;
                        {
                            std::lock_guard<std::mutex> dbLock(
                                DatabaseManager::getInstance().getWriteMutex());
                            ranked = ctx_.skillGraphManager->getTopFreelancers(
                                userID, limit);
                        }

                        nlohmann::json arr = nlohmann::json::array();
                        for (int i = 0; i < ranked.size(); ++i) {
                            const RankedUser& ru = ranked.get(i);
                            std::string name = "Unknown";
                            std::unique_ptr<User> u(
                                ctx_.userManager->findUserByID(ru.userID));
                            if (u) name = u->getName();

                            arr.push_back(
                                sb::api::toRankedUserJson(
                                    ru.userID, name, ru.score, i + 1));
                        }

                        nlohmann::json out;
                        out["rankings"] = arr;
                        out["count"] = ranked.size();
                        sendJson(res, out, 200);
                    }
                    catch (const UnauthorizedException& e) {
                        sendError(res, 403, e.what());
                    }
                    catch (const SkillBridgeException& e) {
                        sendError(res, 500, e.what());
                    }
                    catch (const std::exception& e) {
                        sendError(res, 500,
                            std::string("unexpected error: ") + e.what());
                    }
                });

            // ----- GET /api/endorsements/trusted ---------------------------------
            impl_->server.Get("/api/endorsements/trusted",
                [this](const httplib::Request& req, httplib::Response& res) {
                    int userID = requireAuth(req, res);
                    if (userID < 0) return;

                    try {
                        int maxHops = 2;
                        if (req.has_param("maxHops")) {
                            try {
                                maxHops = std::stoi(
                                    req.get_param_value("maxHops"));
                            }
                            catch (...) {
                                maxHops = 2;
                            }
                        }
                        if (maxHops < 1) maxHops = 1;
                        if (maxHops > 5) maxHops = 5;

                        DataList<TrustedUser> trusted;
                        {
                            std::lock_guard<std::mutex> dbLock(
                                DatabaseManager::getInstance().getWriteMutex());
                            trusted = ctx_.skillGraphManager->getTrustedNear(
                                userID, maxHops);
                        }

                        nlohmann::json arr = nlohmann::json::array();
                        for (int i = 0; i < trusted.size(); ++i) {
                            const TrustedUser& tu = trusted.get(i);
                            std::string name = "Unknown";
                            std::unique_ptr<User> u(
                                ctx_.userManager->findUserByID(tu.userID));
                            if (u) name = u->getName();

                            arr.push_back(
                                sb::api::toTrustedUserJson(
                                    tu.userID, name, tu.hopCount));
                        }

                        nlohmann::json out;
                        out["trusted"] = arr;
                        out["count"] = trusted.size();
                        sendJson(res, out, 200);
                    }
                    catch (const UnauthorizedException& e) {
                        sendError(res, 403, e.what());
                    }
                    catch (const SkillBridgeException& e) {
                        sendError(res, 500, e.what());
                    }
                    catch (const std::exception& e) {
                        sendError(res, 500,
                            std::string("unexpected error: ") + e.what());
                    }
                });
        }

        void HttpServer::registerAdminRoutes() {

            auto requireAdmin = [this](const httplib::Request& req,
                httplib::Response& res) -> int {
                    int userID = requireAuth(req, res);
                    if (userID < 0) return -1;

                    std::unique_ptr<User> viewer(
                        ctx_.userManager->findUserByID(userID));
                    if (!viewer || viewer->getRole() != UserRole::ADMIN) {
                        sendError(res, 403, "admin access required");
                        return -1;
                    }
                    return userID;
                };

            // ----- GET /api/admin/users ------------------------------------------
            impl_->server.Get("/api/admin/users",
                [this, requireAdmin](const httplib::Request& req,
                    httplib::Response& res) {
                        int adminID = requireAdmin(req, res);
                        if (adminID < 0) return;

                        try {
                            DataList<User*> users =
                                ctx_.userManager->adminListAllUsers(adminID);

                            nlohmann::json arr = nlohmann::json::array();
                            for (int i = 0; i < users.size(); ++i) {
                                arr.push_back(sb::api::toJson(*users.get(i)));
                            }

                            // Clean up owned pointers
                            for (int i = 0; i < users.size(); ++i) {
                                delete users.get(i);
                            }

                            nlohmann::json out;
                            out["users"] = arr;
                            out["count"] = static_cast<int>(arr.size());
                            sendJson(res, out, 200);
                        }
                        catch (const UnauthorizedException& e) {
                            sendError(res, 403, e.what());
                        }
                        catch (const SkillBridgeException& e) {
                            sendError(res, 500, e.what());
                        }
                        catch (const std::exception& e) {
                            sendError(res, 500,
                                std::string("unexpected error: ") + e.what());
                        }
                });

            // ----- DELETE /api/admin/users/:id -----------------------------------
            impl_->server.Delete(R"(/api/admin/users/(\d+))",
                [this, requireAdmin](const httplib::Request& req,
                    httplib::Response& res) {
                        int adminID = requireAdmin(req, res);
                        if (adminID < 0) return;

                        try {
                            int targetID = 0;
                            if (!parsePositiveInt(req.matches[1], targetID)) {
                                sendError(res, 400, "invalid user id");
                                return;
                            }

                            std::lock_guard<std::mutex> dbLock(
                                DatabaseManager::getInstance().getWriteMutex());

                            ctx_.userManager->adminDeleteUser(adminID, targetID);

                            // Invalidate ALL sessions for the deleted user
                            {
                                std::lock_guard<std::mutex> sessLock(
                                    impl_->sessionMutex);
                                for (auto it = impl_->sessions.begin();
                                    it != impl_->sessions.end();) {
                                    if (it->second == targetID) {
                                        it = impl_->sessions.erase(it);
                                    }
                                    else {
                                        ++it;
                                    }
                                }
                            }

                            nlohmann::json out;
                            out["ok"] = true;
                            sendJson(res, out, 200);
                        }
                        catch (const ValidationException& e) {
                            sendError(res, 400, e.what());
                        }
                        catch (const UnauthorizedException& e) {
                            sendError(res, 403, e.what());
                        }
                        catch (const SkillBridgeException& e) {
                            sendError(res, 500, e.what());
                        }
                        catch (const std::exception& e) {
                            sendError(res, 500,
                                std::string("unexpected error: ") + e.what());
                        }
                });

            // ----- GET /api/admin/gigs -------------------------------------------
            impl_->server.Get("/api/admin/gigs",
                [this, requireAdmin](const httplib::Request& req,
                    httplib::Response& res) {
                        int adminID = requireAdmin(req, res);
                        if (adminID < 0) return;

                        try {
                            DataList<Gig> gigs =
                                ctx_.gigManager->findAllGigs(adminID);

                            nlohmann::json arr = nlohmann::json::array();
                            for (int i = 0; i < gigs.size(); ++i) {
                                const Gig& g = gigs.get(i);

                                std::string ownerName = "Unknown";
                                std::unique_ptr<User> owner(
                                    ctx_.userManager->findUserByID(g.getOwnerID()));
                                if (owner) ownerName = owner->getName();

                                arr.push_back(
                                    sb::api::toAdminGigJson(g, ownerName));
                            }

                            nlohmann::json out;
                            out["gigs"] = arr;
                            out["count"] = static_cast<int>(arr.size());
                            sendJson(res, out, 200);
                        }
                        catch (const UnauthorizedException& e) 
                        {
                            sendError(res, 403, e.what());
                        }
                        catch (const SkillBridgeException& e) 
                        {
                            sendError(res, 500, e.what());
                        }
                        catch (const std::exception& e) 
                        {
                            sendError(res, 500,
                                std::string("unexpected error: ") + e.what());
                        }
                });

            // ----- DELETE /api/admin/reviews/:id ---------------------------------
            impl_->server.Delete(R"(/api/admin/reviews/(\d+))",
                [this, requireAdmin](const httplib::Request& req, httplib::Response& res) 
                {
                        int adminID = requireAdmin(req, res);
                        if (adminID < 0) 
                            return;

                        try 
                        {
                            int reviewID = 0;
                            if (!parsePositiveInt(req.matches[1], reviewID)) 
                            {
                                sendError(res, 400, "invalid review id");
                                return;
                            }

                            std::lock_guard<std::mutex> dbLock(DatabaseManager::getInstance().getWriteMutex());

                            ctx_.reviewManager->adminDeleteReview(adminID, reviewID);

                            nlohmann::json out;
                            out["ok"] = true;
                            sendJson(res, out, 200);
                        }
                        catch (const UnauthorizedException& e) 
                        {
                            sendError(res, 403, e.what());
                        }
                        catch (const SkillBridgeException& e) 
                        {
                            sendError(res, 404, e.what());
                        }
                        catch (const std::exception& e) 
                        {
                            sendError(res, 500, std::string("unexpected error: ") + e.what());
                        }
                });
        }

    }
} 