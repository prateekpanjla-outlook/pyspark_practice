#ifndef SESSION_MANAGER_HPP
#define SESSION_MANAGER_HPP

#include <string>
#include <unordered_map>
#include <shared_mutex>
#include <chrono>
#include <memory>
#include <vector>
#include "sql_executor.hpp"

namespace sql_practice {

/**
 * @brief Represents a single user session
 *
 * Memory footprint: ~1KB per session (vs 200MB+ for Docker)
 */
struct UserSession {
    std::string user_id;
    std::string session_token;
    std::unique_ptr<DuckDBConnection> db_conn;
    std::chrono::steady_clock::time_point last_activity;
    int query_count;
    std::string current_question_id;  // Track which question's schema is loaded

    UserSession(const std::string& uid, const std::string& token)
        : user_id(uid), session_token(token), query_count(0), current_question_id("") {
        last_activity = std::chrono::steady_clock::now();
    }

    bool is_expired(int timeout_seconds = 120) const {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - last_activity);
        return elapsed.count() > timeout_seconds;
    }

    void update_activity() {
        last_activity = std::chrono::steady_clock::now();
    }
};

/**
 * @brief Manages all active user sessions
 *
 * Thread-safe session management with automatic cleanup
 */
class SessionManager {
private:
    std::unordered_map<std::string, std::shared_ptr<UserSession>> sessions;
    mutable std::shared_mutex sessions_mutex;
    int session_timeout_seconds;

public:
    explicit SessionManager(int timeout_sec = 120)
        : session_timeout_seconds(timeout_sec) {}

    /**
     * @brief Create a new session for a user
     */
    std::string create_session(const std::string& user_id);

    /**
     * @brief Get session by token (thread-safe)
     */
    std::shared_ptr<UserSession> get_session(const std::string& token);

    /**
     * @brief Remove expired sessions (should be called periodically)
     */
    size_t cleanup_expired();

    /**
     * @brief Get current active session count
     */
    size_t get_active_count() const {
        std::shared_lock lock(sessions_mutex);
        return sessions.size();
    }

    /**
     * @brief Terminate a specific session
     */
    void terminate_session(const std::string& token);
};

} // namespace sql_practice

#endif // SESSION_MANAGER_HPP
