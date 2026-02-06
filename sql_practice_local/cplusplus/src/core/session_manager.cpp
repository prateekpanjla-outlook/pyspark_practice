#include "include/session_manager.hpp"
#include "include/sql_executor.hpp"
#include <random>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <mutex>

namespace sql_practice {

std::string SessionManager::create_session(const std::string& user_id) {
    // Generate unique session token
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);

    std::stringstream ss;
    ss << "sess_";
    for (int i = 0; i < 32; ++i) {
        ss << std::hex << dis(gen);
    }
    std::string token = ss.str();

    // Create session
    SQLExecutor executor;
    auto session = std::make_shared<UserSession>(user_id, token);
    session->db_conn = executor.create_connection();

    // Store session
    {
        std::unique_lock lock(sessions_mutex);
        sessions[token] = session;
    }

    return token;
}

std::shared_ptr<UserSession> SessionManager::get_session(const std::string& token) {
    std::shared_lock lock(sessions_mutex);
    auto it = sessions.find(token);
    if (it != sessions.end()) {
        return it->second;
    }
    return nullptr;
}

size_t SessionManager::cleanup_expired() {
    std::vector<std::string> expired_tokens;

    // Find expired sessions
    {
        std::shared_lock lock(sessions_mutex);
        for (const auto& [token, session] : sessions) {
            if (session->is_expired(session_timeout_seconds)) {
                expired_tokens.push_back(token);
            }
        }
    }

    // Remove expired sessions
    {
        std::unique_lock lock(sessions_mutex);
        for (const auto& token : expired_tokens) {
            sessions.erase(token);
        }
    }

    return expired_tokens.size();
}

void SessionManager::terminate_session(const std::string& token) {
    std::unique_lock lock(sessions_mutex);
    sessions.erase(token);
}

} // namespace sql_practice
