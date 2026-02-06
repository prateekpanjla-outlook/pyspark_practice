#include "include/session_manager.hpp"
#include <fstream>
#include <sstream>

namespace sql_practice {

namespace Config {

// Default configuration
int session_timeout_seconds = 120;
int max_concurrent_sessions = 10000;
int server_port = 8080;
int thread_pool_size = 32;
std::string log_level = "info";

// =============================================================================
// TODO: Shared DuckDB Instance Architecture
// =============================================================================
// Current Issue: Each session creates its own DuckDB instance (~24MB virtual memory each)
// With 1000 sessions = 24GB+ virtual memory (though actual RSS is much lower)
//
// Proposed Architecture:
// - Create N shared DuckDB instances (where N = total_sessions / CONNECTIONS_PER_INSTANCE)
// - Each session gets a Connection to one of the shared instances
// - DuckDB can handle many concurrent connections per instance
//
// Configurable constant for connections per DuckDB instance
// Adjust based on benchmarking: test values 100, 250, 500, 1000
// To determine optimal value: Measure response time, CPU, memory, errors
constexpr int CONNECTIONS_PER_INSTANCE = 500;
//
// Example calculation:
// - Target: 10,000 concurrent users
// - Connections per instance: 500
// - Instances needed: 10,000 / 500 = 20 instances
// - Memory: 20 * 24MB = 480MB virtual (vs 240GB for separate instances)
//
// Implementation approach:
// 1. Create connection pool managing multiple DuckDB instances
// 2. Assign each session a connection from the pool (round-robin)
// 3. Return connections to pool when session expires
// 4. Use CONNECTIONS_PER_INSTANCE to calculate pool size
// =============================================================================

// Load from environment or config file
void load_config(const std::string& config_file) {
    // Load from environment variables
    if (const char* env_port = std::getenv("PORT")) {
        server_port = std::stoi(env_port);
    }
    if (const char* env_timeout = std::getenv("SESSION_TIMEOUT")) {
        session_timeout_seconds = std::stoi(env_timeout);
    }
    if (const char* env_max_sessions = std::getenv("MAX_SESSIONS")) {
        max_concurrent_sessions = std::stoi(env_max_sessions);
    }
    if (const char* env_threads = std::getenv("THREAD_POOL_SIZE")) {
        thread_pool_size = std::stoi(env_threads);
    }
    if (const char* env_log = std::getenv("LOG_LEVEL")) {
        log_level = env_log;
    }

    // Optionally load from file
    if (!config_file.empty()) {
        std::ifstream file(config_file);
        if (file.is_open()) {
            std::string line;
            while (std::getline(file, line)) {
                if (line.empty() || line[0] == '#') continue;

                size_t pos = line.find('=');
                if (pos != std::string::npos) {
                    std::string key = line.substr(0, pos);
                    std::string value = line.substr(pos + 1);

                    // Trim whitespace
                    key.erase(0, key.find_first_not_of(" \t"));
                    key.erase(key.find_last_not_of(" \t") + 1);
                    value.erase(0, value.find_first_not_of(" \t"));
                    value.erase(value.find_last_not_of(" \t") + 1);

                    if (key == "PORT") server_port = std::stoi(value);
                    else if (key == "SESSION_TIMEOUT") session_timeout_seconds = std::stoi(value);
                    else if (key == "MAX_SESSIONS") max_concurrent_sessions = std::stoi(value);
                    else if (key == "THREAD_POOL_SIZE") thread_pool_size = std::stoi(value);
                    else if (key == "LOG_LEVEL") log_level = value;
                }
            }
        }
    }
}

} // namespace Config

} // namespace sql_practice
