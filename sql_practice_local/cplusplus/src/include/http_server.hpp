#ifndef HTTP_SERVER_HPP
#define HTTP_SERVER_HPP

#include "session_manager.hpp"
#include "question_loader.hpp"
#include <oatpp/web/server/HttpConnectionHandler.hpp>
#include <oatpp/web/server/HttpRouter.hpp>
#include <oatpp/network/Server.hpp>
#include <oatpp/core/macro/component.hpp>
#include <oatpp/core/macro/codegen.hpp>
#include <memory>

namespace sql_practice {

/**
 * @brief HTTP request/response DTOs
 */
#include OATPP_CODEGEN_BEGIN(DTO)

class LoginRequest : public oatpp::DTO {
    DTO_INIT(LoginRequest, DTO)

    DTO_FIELD(String, user_id);
};

class ExecuteRequest : public oatpp::DTO {
    DTO_INIT(ExecuteRequest, DTO)

    DTO_FIELD(String, session_token);
    DTO_FIELD(String, question_id);
    DTO_FIELD(String, user_sql);
};

class ExecuteResponse : public oatpp::DTO {
    DTO_INIT(ExecuteResponse, DTO)

    DTO_FIELD(List<String>, columns);
    DTO_FIELD(List<Fields<Any>>, rows);
    DTO_FIELD(Boolean, is_correct);
    DTO_FIELD(Int64, execution_time_ms);
    DTO_FIELD(String, error);
};

class QuestionResponse : public oatpp::DTO {
    DTO_INIT(QuestionResponse, DTO)

    DTO_FIELD(String, id);
    DTO_FIELD(String, title);
    DTO_FIELD(String, slug);
    DTO_FIELD(String, description);
    DTO_FIELD(String, question_difficulty);
    DTO_FIELD(String, question_category);
    DTO_FIELD(String, company);
    DTO_FIELD(List<String>, tags);
};

class QuestionDetailResponse : public QuestionResponse {
    DTO_INIT(QuestionDetailResponse, QuestionResponse)

    DTO_FIELD(Fields<Any>, schema_data);
    DTO_FIELD(List<String>, hints);
    DTO_FIELD(String, starter_code);
};

#include OATPP_CODEGEN_END(DTO)

/**
 * @brief HTTP Server component
 *
 * Handles all API endpoints using Oat++ 1.3.0
 */
class HTTPServer {
private:
    std::shared_ptr<SessionManager> session_manager;
    std::shared_ptr<QuestionLoader> question_loader;
    std::shared_ptr<oatpp::network::Server> server;
    std::shared_ptr<oatpp::web::server::HttpRouter> router;

public:
    HTTPServer(
        std::shared_ptr<SessionManager> sm,
        std::shared_ptr<QuestionLoader> ql
    );

    /**
     * @brief Start the HTTP server
     */
    void run(uint16_t port = 8080);

    /**
     * @brief Stop the server
     */
    void stop();

private:
    /**
     * @brief Setup all routes
     */
    void setupRoutes();
};

/**
 * @brief Request handlers
 */
class Handlers {
private:
    static std::shared_ptr<SessionManager> session_manager;
    static std::shared_ptr<QuestionLoader> question_loader;

public:
    static void init(
        std::shared_ptr<SessionManager> sm,
        std::shared_ptr<QuestionLoader> ql
    );

    /**
     * @brief POST /api/login - Create new session
     */
    static oatpp::Object<ExecuteResponse> login(const std::shared_ptr<LoginRequest>& req);

    /**
     * @brief POST /api/execute - Execute SQL query
     */
    static oatpp::Object<ExecuteResponse> execute(const std::shared_ptr<ExecuteRequest>& req);

    /**
     * @brief GET /api/questions - List all questions
     */
    static oatpp::List<oatpp::Object<QuestionResponse>> listQuestions(
        const oatpp::String& difficulty,
        const oatpp::String& category,
        const oatpp::String& tag,
        const oatpp::Int32& skip,
        const oatpp::Int32& limit
    );

    /**
     * @brief GET /api/questions/:slug - Get question details
     */
    static oatpp::Object<QuestionDetailResponse> getQuestion(const oatpp::String& slug);

    /**
     * @brief GET /health - Health check
     */
    static oatpp::String health();
};

} // namespace sql_practice

#endif // HTTP_SERVER_HPP
