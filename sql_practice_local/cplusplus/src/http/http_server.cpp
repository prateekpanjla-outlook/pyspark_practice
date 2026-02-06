#include "include/http_server.hpp"
#include "include/session_manager.hpp"
#include "include/question_loader.hpp"
#include "include/sql_executor.hpp"
#include <oatpp/web/server/HttpConnectionHandler.hpp>
#include <oatpp/web/server/HttpRouter.hpp>
#include <oatpp/web/protocol/http/Http.hpp>
#include <oatpp/core/Types.hpp>
#include <oatpp/network/tcp/server/ConnectionProvider.hpp>
#include <sstream>
#include <iostream>
#include <functional>
#include <fstream>

namespace sql_practice {

// =============================================================================
// Request Handlers using Oat++ 1.3.0 API
// =============================================================================

/**
 * @brief Custom RequestHandler for health endpoint
 */
class HealthHandler : public oatpp::web::server::HttpRequestHandler {
private:
    std::shared_ptr<SessionManager> session_manager;
    std::shared_ptr<QuestionLoader> question_loader;
public:
    HealthHandler(std::shared_ptr<SessionManager> sm,
                 std::shared_ptr<QuestionLoader> ql)
        : session_manager(sm), question_loader(ql) {}

    std::shared_ptr<oatpp::web::protocol::http::outgoing::Response> handle(
        const std::shared_ptr<oatpp::web::protocol::http::incoming::Request>& request) override {

        size_t active = session_manager ? session_manager->get_active_count() : 0;
        size_t total = question_loader ? question_loader->get_count() : 0;

        char buffer[256];
        snprintf(buffer, sizeof(buffer),
                 "{\"status\":\"healthy\",\"active_sessions\":%zu,\"total_questions\":%zu}",
                 active, total);

        auto body = oatpp::String(buffer);
        return oatpp::web::protocol::http::outgoing::ResponseFactory::createResponse(
            oatpp::web::protocol::http::Status::CODE_200, body
        );
    }
};

/**
 * @brief Custom RequestHandler for login endpoint
 */
class LoginHandler : public oatpp::web::server::HttpRequestHandler {
private:
    std::shared_ptr<SessionManager> session_manager;
public:
    LoginHandler(std::shared_ptr<SessionManager> sm) : session_manager(sm) {}

    std::shared_ptr<oatpp::web::protocol::http::outgoing::Response> handle(
        const std::shared_ptr<oatpp::web::protocol::http::incoming::Request>& request) override {

        try {
            // Read request body
            auto body_str = request->readBodyToString();
            if (!body_str || body_str->empty()) {
                auto dto = oatpp::String("{\"is_correct\":false,\"error\":\"Request body is required\"}");
                return oatpp::web::protocol::http::outgoing::ResponseFactory::createResponse(
                    oatpp::web::protocol::http::Status::CODE_400, dto
                );
            }

            // Parse JSON (simple parsing for user_id)
            std::string body = body_str->c_str();
            size_t user_id_pos = body.find("\"user_id\":");
            if (user_id_pos == std::string::npos) {
                auto dto = oatpp::String("{\"is_correct\":false,\"error\":\"user_id is required\"}");
                return oatpp::web::protocol::http::outgoing::ResponseFactory::createResponse(
                    oatpp::web::protocol::http::Status::CODE_400, dto
                );
            }

            // Extract user_id value
            size_t start = body.find("\"", user_id_pos + 10);
            size_t end = body.find("\"", start + 1);
            if (start == std::string::npos || end == std::string::npos) {
                auto dto = oatpp::String("{\"is_correct\":false,\"error\":\"Invalid user_id format\"}");
                return oatpp::web::protocol::http::outgoing::ResponseFactory::createResponse(
                    oatpp::web::protocol::http::Status::CODE_400, dto
                );
            }

            std::string user_id = body.substr(start + 1, end - start - 1);

            // Create session
            std::string session_token = session_manager->create_session(user_id);

            // Return response with session token
            std::stringstream json;
            json << "{"
                 << "\"is_correct\":true,"
                 << "\"error\":\"\","
                 << "\"session_token\":\"" << session_token << "\""
                 << "}";

            auto dto = oatpp::String(json.str());
            return oatpp::web::protocol::http::outgoing::ResponseFactory::createResponse(
                oatpp::web::protocol::http::Status::CODE_200, dto
            );

        } catch (const std::exception& e) {
            auto dto = oatpp::String(std::string("{\"is_correct\":false,\"error\":\"") + e.what() + "\"}");
            return oatpp::web::protocol::http::outgoing::ResponseFactory::createResponse(
                oatpp::web::protocol::http::Status::CODE_500, dto
            );
        }
    }
};

/**
 * @brief Custom RequestHandler for execute endpoint
 */
class ExecuteHandler : public oatpp::web::server::HttpRequestHandler {
private:
    std::shared_ptr<SessionManager> session_manager;
    std::shared_ptr<QuestionLoader> question_loader;

public:
    ExecuteHandler(std::shared_ptr<SessionManager> sm,
                   std::shared_ptr<QuestionLoader> ql)
        : session_manager(sm), question_loader(ql) {}

    std::shared_ptr<oatpp::web::protocol::http::outgoing::Response> handle(
        const std::shared_ptr<oatpp::web::protocol::http::incoming::Request>& request) override {

        try {
            // Read request body
            auto body_str = request->readBodyToString();
            if (!body_str || body_str->empty()) {
                auto dto = oatpp::String("{\"is_correct\":false,\"error\":\"Request body is required\"}");
                return oatpp::web::protocol::http::outgoing::ResponseFactory::createResponse(
                    oatpp::web::protocol::http::Status::CODE_400, dto
                );
            }

            std::string body = body_str->c_str();

            // Parse session_token
            std::string session_token;
            size_t token_pos = body.find("\"session_token\":");
            if (token_pos != std::string::npos) {
                size_t start = body.find("\"", token_pos + 17);
                size_t end = body.find("\"", start + 1);
                if (start != std::string::npos && end != std::string::npos) {
                    session_token = body.substr(start + 1, end - start - 1);
                }
            }

            // Validate session
            if (session_token.empty()) {
                auto dto = oatpp::String("{\"is_correct\":false,\"error\":\"session_token is required\"}");
                return oatpp::web::protocol::http::outgoing::ResponseFactory::createResponse(
                    oatpp::web::protocol::http::Status::CODE_400, dto
                );
            }

            auto session = session_manager->get_session(session_token);
            if (!session || session->is_expired()) {
                auto dto = oatpp::String("{\"is_correct\":false,\"error\":\"Invalid or expired session\"}");
                return oatpp::web::protocol::http::outgoing::ResponseFactory::createResponse(
                    oatpp::web::protocol::http::Status::CODE_401, dto
                );
            }

            // Update activity
            session->update_activity();
            session->query_count++;

            // Parse user_sql
            std::string user_sql;
            size_t sql_pos = body.find("\"user_sql\":");
            if (sql_pos != std::string::npos) {
                size_t start = body.find("\"", sql_pos + 12);
                size_t end = body.find("\"", start + 1);
                if (start != std::string::npos && end != std::string::npos) {
                    std::string sql_escaped = body.substr(start + 1, end - start - 1);
                    // Unescape JSON string (simple version)
                    user_sql = sql_escaped;
                    // Replace escaped quotes
                    size_t pos = 0;
                    while ((pos = user_sql.find("\\\"", pos)) != std::string::npos) {
                        user_sql.replace(pos, 2, "\"");
                    }
                }
            }

            // Parse question_id (for result comparison)
            std::string question_id;
            size_t qid_pos = body.find("\"question_id\":");
            if (qid_pos != std::string::npos) {
                size_t start = body.find("\"", qid_pos + 14);
                size_t end = body.find("\"", start + 1);
                if (start != std::string::npos && end != std::string::npos) {
                    question_id = body.substr(start + 1, end - start - 1);
                }
            }

            if (user_sql.empty()) {
                auto dto = oatpp::String("{\"is_correct\":false,\"error\":\"user_sql is required\"}");
                return oatpp::web::protocol::http::outgoing::ResponseFactory::createResponse(
                    oatpp::web::protocol::http::Status::CODE_400, dto
                );
            }

            // Execute SQL
            SQLExecutor executor;
            auto result = executor.execute(session->db_conn.get(), user_sql);

            if (!result.success) {
                auto dto = oatpp::String("{\"is_correct\":false,\"error\":\"" + result.error_message + "\"}");
                return oatpp::web::protocol::http::outgoing::ResponseFactory::createResponse(
                    oatpp::web::protocol::http::Status::CODE_400, dto
                );
            }

            // Compare with expected result if question_id is provided
            bool is_correct = true;
            if (!question_id.empty()) {
                auto question = question_loader->get_question_by_id(question_id);
                if (question && question->expected_output.success) {
                    // Compare column names
                    if (result.columns != question->expected_output.columns) {
                        is_correct = false;
                    }
                    // Compare row count
                    else if (result.rows.size() != question->expected_output.rows.size()) {
                        is_correct = false;
                    }
                    // Compare actual data
                    else {
                        for (const auto& expected_row : question->expected_output.rows) {
                            bool row_found = false;
                            for (const auto& actual_row : result.rows) {
                                if (actual_row == expected_row) {
                                    row_found = true;
                                    break;
                                }
                            }
                            if (!row_found) {
                                is_correct = false;
                                break;
                            }
                        }
                    }
                }
            }

            // Build response with columns and rows
            std::stringstream json;
            json << "{"
                 << "\"is_correct\":" << (is_correct ? "true" : "false") << ","
                 << "\"execution_time_ms\":" << result.execution_time_ms << ","
                 << "\"columns\":[";

            // Add columns
            for (size_t i = 0; i < result.columns.size(); ++i) {
                if (i > 0) json << ",";
                json << "\"" << result.columns[i] << "\"";
            }

            json << "],"
                 << "\"rows\":[";

            // Add rows
            for (size_t i = 0; i < result.rows.size(); ++i) {
                if (i > 0) json << ",";
                json << "{";
                const auto& row = result.rows[i];
                bool first = true;
                for (const auto& col : result.columns) {
                    if (!first) json << ",";
                    json << "\"" << col << "\":";

                    auto it = row.find(col);
                    if (it != row.end()) {
                        // Escape the value for JSON
                        std::string val = it->second;
                        // Simple JSON escaping
                        std::string escaped;
                        for (char c : val) {
                            if (c == '"') escaped += "\\\"";
                            else if (c == '\\') escaped += "\\\\";
                            else if (c == '\n') escaped += "\\n";
                            else if (c == '\r') escaped += "\\r";
                            else if (c == '\t') escaped += "\\t";
                            else escaped += c;
                        }
                        json << "\"" << escaped << "\"";
                    } else {
                        json << "null";
                    }
                    first = false;
                }
                json << "}";
            }

            json << "],"
                 << "\"execution_time_ms\":" << result.execution_time_ms
                 << "}";

            auto dto = oatpp::String(json.str());
            return oatpp::web::protocol::http::outgoing::ResponseFactory::createResponse(
                oatpp::web::protocol::http::Status::CODE_200, dto
            );

        } catch (const std::exception& e) {
            auto dto = oatpp::String(std::string("{\"is_correct\":false,\"error\":\"") + e.what() + "\"}");
            return oatpp::web::protocol::http::outgoing::ResponseFactory::createResponse(
                oatpp::web::protocol::http::Status::CODE_500, dto
            );
        }
    }
};

/**
 * @brief Custom RequestHandler for list questions endpoint
 */
class ListQuestionsHandler : public oatpp::web::server::HttpRequestHandler {
private:
    std::shared_ptr<QuestionLoader> question_loader;
public:
    ListQuestionsHandler(std::shared_ptr<QuestionLoader> ql) : question_loader(ql) {}

    std::shared_ptr<oatpp::web::protocol::http::outgoing::Response> handle(
        const std::shared_ptr<oatpp::web::protocol::http::incoming::Request>& request) override {

        // Get query parameters using Oat++ 1.3.0 API
        auto queryParams = request->getQueryParameters();
        auto allParams = queryParams.getAll();

        // Helper lambda to get parameter with default
        auto getParam = [&](const char* name, const char* defaultVal) -> std::string {
            auto key = oatpp::data::share::StringKeyLabel(name);
            auto it = allParams.find(key);
            if (it != allParams.end()) {
                return it->second.std_str();
            }
            return defaultVal;
        };

        std::string difficulty = getParam("difficulty", "medium");
        std::string category = getParam("category", "sql");
        std::string tag = getParam("tag", "");
        std::string skip_str = getParam("skip", "0");
        std::string limit_str = getParam("limit", "20");

        std::string diff_str = difficulty;
        std::string cat_str = category;
        std::string tag_str = tag;
        int skip_val = skip_str.empty() ? 0 : std::stoi(skip_str);
        int limit_val = limit_str.empty() ? 20 : std::stoi(limit_str);

        auto questions = question_loader->list_questions(diff_str, cat_str, tag_str, skip_val, limit_val);

        // Build JSON response manually
        std::stringstream json;
        json << "[";
        for (size_t i = 0; i < questions.size(); ++i) {
            if (i > 0) json << ",";
            const auto& q = questions[i];
            json << "{"
                 << "\"id\":\"" << q.id << "\","
                 << "\"title\":\"" << q.title << "\","
                 << "\"slug\":\"" << q.slug << "\","
                 << "\"description\":\"" << q.description << "\","
                 << "\"difficulty\":\"" << q.question_difficulty << "\","
                 << "\"category\":\"" << q.category << "\""
                 << "}";
        }
        json << "]";

        auto dto = oatpp::String(json.str());
        return oatpp::web::protocol::http::outgoing::ResponseFactory::createResponse(
            oatpp::web::protocol::http::Status::CODE_200, dto
        );
    }
};

/**
 * @brief Custom RequestHandler for get question endpoint
 */
class GetQuestionHandler : public oatpp::web::server::HttpRequestHandler {
private:
    std::shared_ptr<QuestionLoader> question_loader;
public:
    GetQuestionHandler(std::shared_ptr<QuestionLoader> ql) : question_loader(ql) {}

    std::shared_ptr<oatpp::web::protocol::http::outgoing::Response> handle(
        const std::shared_ptr<oatpp::web::protocol::http::incoming::Request>& request) override {

        // Get path tail (the part after /api/questions/)
        auto path_tail = request->getPathTail();
        std::string slug = path_tail ? path_tail->c_str() : "";

        if (slug.empty()) {
            auto dto = oatpp::String("{\"error\":\"Not found\"}");
            return oatpp::web::protocol::http::outgoing::ResponseFactory::createResponse(
                oatpp::web::protocol::http::Status::CODE_404, dto
            );
        }

        // Remove leading slash if present
        if (!slug.empty() && slug[0] == '/') {
            slug = slug.substr(1);
        }

        auto question = question_loader->get_question_by_slug(slug);

        if (!question) {
            auto dto = oatpp::String("{\"error\":\"Question not found\"}");
            return oatpp::web::protocol::http::outgoing::ResponseFactory::createResponse(
                oatpp::web::protocol::http::Status::CODE_404, dto
            );
        }

        const auto& q = *question;
        std::stringstream json;
        json << "{"
             << "\"id\":\"" << q.id << "\","
             << "\"title\":\"" << q.title << "\","
             << "\"slug\":\"" << q.slug << "\","
             << "\"description\":\"" << q.description << "\","
             << "\"difficulty\":\"" << q.question_difficulty << "\","
             << "\"category\":\"" << q.category << "\","
             << "\"starter_code\":\"" << q.starter_code << "\""
             << "}";

        auto dto = oatpp::String(json.str());
        return oatpp::web::protocol::http::outgoing::ResponseFactory::createResponse(
            oatpp::web::protocol::http::Status::CODE_200, dto
        );
    }
};

/**
 * @brief Handler for serving static files
 */
class StaticFileHandler : public oatpp::web::server::HttpRequestHandler {
private:
    std::string base_path;

    static std::string getMimeType(const std::string& path) {
        if (path.size() >= 5 && path.compare(path.size() - 5, 5, ".html") == 0) return "text/html";
        if (path.size() >= 4 && path.compare(path.size() - 4, 4, ".css") == 0) return "text/css";
        if (path.size() >= 3 && path.compare(path.size() - 3, 3, ".js") == 0) return "application/javascript";
        if (path.size() >= 5 && path.compare(path.size() - 5, 5, ".json") == 0) return "application/json";
        if (path.size() >= 4 && path.compare(path.size() - 4, 4, ".png") == 0) return "image/png";
        if ((path.size() >= 4 && path.compare(path.size() - 4, 4, ".jpg") == 0) ||
            (path.size() >= 5 && path.compare(path.size() - 5, 5, ".jpeg") == 0)) return "image/jpeg";
        if (path.size() >= 4 && path.compare(path.size() - 4, 4, ".svg") == 0) return "image/svg+xml";
        return "text/plain";
    }

public:
    StaticFileHandler(const std::string& path) : base_path(path) {}

    std::shared_ptr<oatpp::web::protocol::http::outgoing::Response> handle(
        const std::shared_ptr<oatpp::web::protocol::http::incoming::Request>& request) override {

        auto path = request->getPathTail();
        std::string pathStr = path ? path->c_str() : "";

        // Default to index.html for root
        if (pathStr.empty() || pathStr == "/") {
            pathStr = "/index.html";
        }

        // Remove leading slash
        if (!pathStr.empty() && pathStr[0] == '/') {
            pathStr = pathStr.substr(1);
        }

        std::string fullPath = base_path + "/" + pathStr;

        // Read file
        std::ifstream file(fullPath, std::ios::binary);
        if (!file.is_open()) {
            auto dto = oatpp::String("404 - File not found");
            return oatpp::web::protocol::http::outgoing::ResponseFactory::createResponse(
                oatpp::web::protocol::http::Status::CODE_404, dto
            );
        }

        // Read file content
        std::string content((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());

        auto body = oatpp::String(content);
        auto response = oatpp::web::protocol::http::outgoing::ResponseFactory::createResponse(
            oatpp::web::protocol::http::Status::CODE_200, body
        );

        // Set content type
        response->putHeader("Content-Type", getMimeType(pathStr));

        return response;
    }
};

// =============================================================================
// HTTPServer Implementation
// =============================================================================

HTTPServer::HTTPServer(
    std::shared_ptr<SessionManager> sm,
    std::shared_ptr<QuestionLoader> ql
) : session_manager(sm), question_loader(ql) {
    router = oatpp::web::server::HttpRouter::createShared();
    setupRoutes();
}

void HTTPServer::setupRoutes() {
    // Serve index.html for root path
    router->route("GET", "/", std::make_shared<StaticFileHandler>("/home/vagrant/project/cplusplus/web"));

    // Health check
    router->route("GET", "/health", std::make_shared<HealthHandler>(session_manager, question_loader));

    // Login
    router->route("POST", "/api/login", std::make_shared<LoginHandler>(session_manager));

    // Execute SQL
    router->route("POST", "/api/execute", std::make_shared<ExecuteHandler>(session_manager, question_loader));

    // List questions
    router->route("GET", "/api/questions", std::make_shared<ListQuestionsHandler>(question_loader));

    // Get question by slug
    router->route("GET", "/api/questions/*", std::make_shared<GetQuestionHandler>(question_loader));

    // Static files - serve the web interface (catch-all route, must be last)
    router->route("GET", "/*", std::make_shared<StaticFileHandler>("/home/vagrant/project/cplusplus/web"));
}

void HTTPServer::run(uint16_t port) {
    auto connectionHandler = oatpp::web::server::HttpConnectionHandler::createShared(router);
    auto connectionProvider = oatpp::network::tcp::server::ConnectionProvider::createShared(
        {"0.0.0.0", port}
    );

    server = oatpp::network::Server::createShared(
        connectionProvider,
        connectionHandler
    );

    std::cout << "Server is listening on port " << port << "..." << std::endl;
    server->run();
}

void HTTPServer::stop() {
    if (server) {
        server->stop();
    }
}

// =============================================================================
// Handlers Implementation (for DTO-based endpoints)
// =============================================================================

std::shared_ptr<SessionManager> Handlers::session_manager = nullptr;
std::shared_ptr<QuestionLoader> Handlers::question_loader = nullptr;

void Handlers::init(
    std::shared_ptr<SessionManager> sm,
    std::shared_ptr<QuestionLoader> ql
) {
    session_manager = sm;
    question_loader = ql;
}

oatpp::Object<ExecuteResponse> Handlers::login(const std::shared_ptr<LoginRequest>& req) {
    auto response = ExecuteResponse::createShared();

    if (!req || !req->user_id) {
        response->is_correct = false;
        response->error = "user_id is required";
        return response;
    }

    std::string user_id = req->user_id->c_str();
    std::string session_token = session_manager->create_session(user_id);

    response->is_correct = true;
    response->error = oatpp::String(session_token.c_str());

    return response;
}

oatpp::Object<ExecuteResponse> Handlers::execute(const std::shared_ptr<ExecuteRequest>& req) {
    auto response = ExecuteResponse::createShared();

    if (!req || !req->session_token || !req->user_sql) {
        response->is_correct = false;
        response->error = "session_token and user_sql are required";
        return response;
    }

    std::string token = req->session_token->c_str();
    auto session = session_manager->get_session(token);

    if (!session || session->is_expired()) {
        response->is_correct = false;
        response->error = "Invalid or expired session";
        return response;
    }

    session->update_activity();
    session->query_count++;

    std::string user_sql = req->user_sql->c_str();

    SQLExecutor executor;
    auto result = executor.execute(session->db_conn.get(), user_sql);

    if (!result.success) {
        response->is_correct = false;
        response->error = oatpp::String(result.error_message.c_str());
        response->execution_time_ms = result.execution_time_ms;
        return response;
    }

    auto columns = response->columns;
    for (const auto& col : result.columns) {
        columns->push_back(oatpp::String(col.c_str()));
    }

    auto rows = response->rows;
    for (const auto& row : result.rows) {
        auto rowObj = oatpp::Fields<oatpp::Any>::createShared();
        for (const auto& col : result.columns) {
            auto it = row.find(col);
            if (it != row.end()) {
                rowObj->push_back({oatpp::String(col.c_str()), oatpp::String(it->second.c_str())});
            }
        }
        rows->push_back(rowObj);
    }

    response->is_correct = true;
    response->execution_time_ms = result.execution_time_ms;

    return response;
}

oatpp::List<oatpp::Object<QuestionResponse>> Handlers::listQuestions(
    const oatpp::String& difficulty,
    const oatpp::String& category,
    const oatpp::String& tag,
    const oatpp::Int32& skip,
    const oatpp::Int32& limit
) {
    auto result = oatpp::List<oatpp::Object<QuestionResponse>>::createShared();

    std::string diff_str = difficulty ? difficulty->c_str() : "";
    std::string cat_str = category ? category->c_str() : "";
    std::string tag_str = tag ? tag->c_str() : "";
    int skip_val = skip ? (*skip) : 0;
    int limit_val = limit ? (*limit) : 20;

    auto questions = question_loader->list_questions(diff_str, cat_str, tag_str, skip_val, limit_val);

    for (const auto& q : questions) {
        auto qr = QuestionResponse::createShared();
        qr->id = oatpp::String(q.id.c_str());
        qr->title = oatpp::String(q.title.c_str());
        qr->slug = oatpp::String(q.slug.c_str());
        qr->description = oatpp::String(q.description.c_str());
        qr->question_difficulty = oatpp::String(q.question_difficulty.c_str());
        qr->question_category = oatpp::String(q.category.c_str());
        qr->company = oatpp::String(q.company.c_str());

        auto tags = qr->tags;
        for (const auto& tag : q.tags) {
            tags->push_back(oatpp::String(tag.c_str()));
        }

        result->push_back(qr);
    }

    return result;
}

oatpp::Object<QuestionDetailResponse> Handlers::getQuestion(const oatpp::String& slug) {
    auto response = QuestionDetailResponse::createShared();

    if (!slug) {
        return response;
    }

    std::string slug_str = slug->c_str();
    auto question = question_loader->get_question_by_slug(slug_str);

    if (!question) {
        return response;
    }

    auto q = *question;
    response->id = oatpp::String(q.id.c_str());
    response->title = oatpp::String(q.title.c_str());
    response->slug = oatpp::String(q.slug.c_str());
    response->description = oatpp::String(q.description.c_str());
    response->question_difficulty = oatpp::String(q.question_difficulty.c_str());
    response->question_category = oatpp::String(q.category.c_str());
    response->company = oatpp::String(q.company.c_str());
    response->starter_code = oatpp::String(q.starter_code.c_str());

    auto tags = response->tags;
    for (const auto& tag : q.tags) {
        tags->push_back(oatpp::String(tag.c_str()));
    }

    auto hints = response->hints;
    for (const auto& hint : q.hints) {
        hints->push_back(oatpp::String(hint.c_str()));
    }

    return response;
}

oatpp::String Handlers::health() {
    size_t active = session_manager ? session_manager->get_active_count() : 0;
    size_t total = question_loader ? question_loader->get_count() : 0;

    char buffer[256];
    snprintf(buffer, sizeof(buffer),
        "{\"status\":\"healthy\",\"active_sessions\":%zu,\"total_questions\":%zu}",
        active, total
    );

    return oatpp::String(buffer);
}

} // namespace sql_practice
