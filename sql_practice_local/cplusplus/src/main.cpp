#include <oatpp/core/base/Environment.hpp>
#include <oatpp/network/Server.hpp>
#include <oatpp/web/server/HttpConnectionHandler.hpp>

#include "include/session_manager.hpp"
#include "include/sql_executor.hpp"
#include "include/question_loader.hpp"
#include "include/http_server.hpp"
#include "include/duckdb_instance_manager.hpp"

#include <iostream>
#include <csignal>
#include <memory>
#include <thread>
#include <chrono>

using namespace sql_practice;

// Global pointers for signal handler
std::shared_ptr<HTTPServer> server;
std::shared_ptr<SessionManager> session_manager;
std::shared_ptr<QuestionLoader> question_loader;
std::atomic<bool> running(true);
std::chrono::steady_clock::time_point start_time;

/**
 * @brief Server timeout checker - exits after 10 minutes
 *
 * Checks every second if server has been running for 10 minutes
 */
void timeout_checker() {
    const auto TIMEOUT_DURATION = std::chrono::minutes(10);
    while (running.load()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));

        auto elapsed = std::chrono::steady_clock::now() - start_time;
        if (elapsed >= TIMEOUT_DURATION) {
            std::cout << "⏰ Server timeout reached (10 minutes). Shutting down..." << std::endl;
            running.store(false);
            if (server) {
                server->stop();
            }
            break;
        }
    }
}

/**
 * @brief Cleanup session worker thread
 *
 * Runs every 30 seconds to remove expired sessions
 */
void cleanup_worker() {
    while (running.load()) {
        std::this_thread::sleep_for(std::chrono::seconds(30));

        size_t cleaned = session_manager->cleanup_expired();
        size_t active = session_manager->get_active_count();

        if (cleaned > 0) {
            std::cout << "🧹 Cleaned up " << cleaned << " expired sessions"
                      << " | Active: " << active << std::endl;
        }
    }
}

/**
 * @brief Signal handler for graceful shutdown
 * NOTE: Keep minimal - only set flag and stop server (async-signal-safe)
 * Don't use std::cout or mutexes here!
 */
void signal_handler(int signal) {
    // Set flag for main loop to check
    running.store(false);

    // Stop the server (this should be async-signal-safe)
    if (server) {
        server->stop();
    }
}

/**
 * @brief Print startup banner
 */
void print_banner() {
    std::cout << R"(
╔═══════════════════════════════════════════════════════╗
║   SQL Practice Server - C++ Edition                  ║
║   Monolithic architecture for 10K+ concurrent users  ║
╚═══════════════════════════════════════════════════════╝
)" << std::endl;
}

/**
 * @brief Print configuration
 */
void print_config() {
    std::cout << "⚙️  Configuration:" << std::endl;
    std::cout << "   - Session timeout: 10 minutes" << std::endl;
    std::cout << "   - Database engine: DuckDB (SQL:2003 compliant)" << std::endl;
    std::cout << "   - Embedded questions: " << question_loader->get_count() << std::endl;
    std::cout << "   - Max concurrent users: 10,000+" << std::endl;
    std::cout << std::endl;
}

/**
 * @brief Main entry point
 */
int main(int argc, char** argv) {
    // Initialize Oat++ environment
    oatpp::base::Environment::init();

    print_banner();

    // Parse command line arguments
    uint16_t port = 8080;
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--port" && i + 1 < argc) {
            port = static_cast<uint16_t>(std::stoi(argv[++i]));
        } else if (arg == "--help" || arg == "-h") {
            std::cout << "Usage: " << argv[0] << " [--port PORT] [--help]" << std::endl;
            std::cout << "  --port PORT    Server port (default: 8080)" << std::endl;
            return 0;
        }
    }

    try {
        // Initialize components
        std::cout << "🔧 Initializing components..." << std::endl;

        // 1. Load embedded questions
        question_loader = std::make_shared<QuestionLoader>();
        question_loader->load_embedded_questions();
        std::cout << "   ✅ Questions loaded: " << question_loader->get_count() << std::endl;

        // 2. Initialize all database schemas (before accepting connections)
        std::cout << "   📊 Initializing database schemas..." << std::endl;
        if (!SQLExecutor::initialize_all_schemas(question_loader.get())) {
            std::cerr << "❌ Fatal error: Failed to initialize database schemas" << std::endl;
            return 1;
        }

        // 3. Create session manager (10-min timeout)
        session_manager = std::make_shared<SessionManager>(600);  // 600 seconds (10 minutes)
        std::cout << "   ✅ Session manager initialized" << std::endl;

        // 3. Initialize handlers with dependencies
        Handlers::init(session_manager, question_loader);
        std::cout << "   ✅ HTTP handlers initialized" << std::endl;

        // 4. Create and start HTTP server
        server = std::make_shared<HTTPServer>(session_manager, question_loader);
        std::cout << "   ✅ HTTP server initialized" << std::endl;

        print_config();

        // Setup signal handlers for graceful shutdown
        std::signal(SIGINT, signal_handler);
        std::signal(SIGTERM, signal_handler);

        // Record start time and start timeout checker (10 minute auto-shutdown)
        start_time = std::chrono::steady_clock::now();
        std::thread timeout_thread(timeout_checker);
        timeout_thread.detach();

        // Start cleanup worker thread
        std::thread cleanup_thread(cleanup_worker);
        cleanup_thread.detach();

        // Start server (blocking)
        std::cout << "🚀 Server starting on port " << port << "..." << std::endl;
        std::cout << "   Health check: http://localhost:" << port << "/health" << std::endl;
        std::cout << std::endl;

        server->run(port);

        // Cleanup on shutdown
        std::cout << "🧹 Cleaning up..." << std::endl;
        running.store(false);

        // Print telemetry before shutting down
        DuckDBInstanceManager::get().shutdown();

    } catch (const std::exception& e) {
        std::cerr << "❌ Fatal error: " << e.what() << std::endl;
        oatpp::base::Environment::destroy();
        return 1;
    }

    // Destroy Oat++ environment
    oatpp::base::Environment::destroy();

    std::cout << "✅ Server stopped gracefully" << std::endl;
    return 0;
}
