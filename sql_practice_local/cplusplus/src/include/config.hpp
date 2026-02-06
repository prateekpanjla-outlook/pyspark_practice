#pragma once

#include <string>

namespace sql_practice {
namespace Config {

// Configuration constants loaded at runtime
extern int session_timeout_seconds;
extern int max_concurrent_sessions;
extern int server_port;
extern int thread_pool_size;
extern std::string log_level;

// =============================================================================
// Shared DuckDB Instance Architecture
// =============================================================================
// Configurable: Connections per DuckDB instance
// Adjust based on benchmarking (test: 100, 250, 500, 1000)
constexpr int CONNECTIONS_PER_INSTANCE = 500;

} // namespace Config
} // namespace sql_practice
