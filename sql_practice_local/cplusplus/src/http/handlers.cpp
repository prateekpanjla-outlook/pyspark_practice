#include "include/http_server.hpp"
#include "include/session_manager.hpp"
#include "include/question_loader.hpp"
#include "include/sql_executor.hpp"
#include <oatpp/web/server/api/ApiController.hpp>
#include <oatpp/parser/json/mapping/ObjectMapper.hpp>

namespace sql_practice {

// Static member initialization
std::shared_ptr<SessionManager> Handlers::session_manager = nullptr;
std::shared_ptr<QuestionLoader> Handlers::question_loader = nullptr;

void Handlers::init(
    std::shared_ptr<SessionManager> sm,
    std::shared_ptr<QuestionLoader> ql
) {
    session_manager = sm;
    question_loader = ql;
}

std::shared_ptr<ExecuteResponse> Handlers::login(const std::shared_ptr<LoginRequest>& req) {
    auto response = ExecuteResponse::createShared();

    if (!req || !req->user_id) {
        response->is_correct = false;
        response->error = "user_id is required";
        return response;
    }

    // Create session
    std::string user_id = req->user_id->c_str();
    std::string session_token = session_manager->create_session(user_id);

    response->is_correct = true;
    response->error = session_token;

    return response;
}

std::shared_ptr<ExecuteResponse> Handlers::execute(const std::shared_ptr<ExecuteRequest>& req) {
    auto response = ExecuteResponse::createShared();

    if (!req || !req->session_token || !req->user_sql) {
        response->is_correct = false;
        response->error = "session_token and user_sql are required";
        return response;
    }

    // Get session
    std::string token = req->session_token->c_str();
    auto session = session_manager->get_session(token);

    if (!session || session->is_expired()) {
        response->is_correct = false;
        response->error = "Invalid or expired session";
        return response;
    }

    // Update activity
    session->update_activity();
    session->query_count++;

    // Get question
    std::string question_id = req->question_id ? req->question_id->c_str() : "";
    std::string user_sql = req->user_sql->c_str();

    // Execute SQL
    SQLExecutor executor;
    auto result = executor.execute(session->db_conn.get(), user_sql);

    if (!result.success) {
        response->is_correct = false;
        response->error = result.error_message;
        response->execution_time_ms = result.execution_time_ms;
        return response;
    }

    // Convert to Oat++ types
    auto columns = response->columns;
    for (const auto& col : result.columns) {
        columns->push_back(oatpp::String(col));
    }

    auto rows = response->rows;
    // Note: Simplified row conversion - would need full DTO mapping in production

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
    int skip_val = skip ? skip->getValue() : 0;
    int limit_val = limit ? limit->getValue() : 20;

    auto questions = question_loader->list_questions(diff_str, cat_str, tag_str, skip_val, limit_val);

    for (const auto& q : questions) {
        auto qr = QuestionResponse::createShared();
        qr->id = q.id;
        qr->title = q.title;
        qr->slug = q.slug;
        qr->description = q.description;
        qr->difficulty = q.difficulty;
        qr->category = q.category;
        qr->company = q.company;

        auto tags = qr->tags;
        for (const auto& tag : q.tags) {
            tags->push_back(oatpp::String(tag));
        }

        result->push_back(qr);
    }

    return result;
}

std::shared_ptr<QuestionDetailResponse> Handlers::getQuestion(const oatpp::String& slug) {
    auto response = std::make_shared<QuestionDetailResponse>();

    if (!slug) {
        return response;
    }

    std::string slug_str = slug->c_str();
    auto question = question_loader->get_question_by_slug(slug_str);

    if (!question) {
        return response;
    }

    auto q = *question;
    response->id = q.id;
    response->title = q.title;
    response->slug = q.slug;
    response->description = q.description;
    response->difficulty = q.difficulty;
    response->category = q.category;
    response->company = q.company;
    response->starter_code = q.starter_code;

    auto tags = response->tags;
    for (const auto& tag : q.tags) {
        tags->push_back(oatpp::String(tag));
    }

    auto hints = response->hints;
    for (const auto& hint : q.hints) {
        hints->push_back(oatpp::String(hint));
    }

    // Schema data would be converted here in full implementation

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
