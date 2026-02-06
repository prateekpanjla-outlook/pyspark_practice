#include "include/duckdb_instance_manager.hpp"

namespace sql_practice {

// Static member initialization
std::unique_ptr<DuckDBInstanceManager> DuckDBInstanceManager::instance_ = nullptr;
std::mutex DuckDBInstanceManager::mutex_;

DuckDBInstanceManager::DuckDBInstanceManager() {
    // Private constructor
}

DuckDBInstanceManager& DuckDBInstanceManager::get() {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!instance_) {
        instance_ = std::unique_ptr<DuckDBInstanceManager>(new DuckDBInstanceManager());
    }

    return *instance_;
}

bool DuckDBInstanceManager::initialize(const std::string& path) {
    std::lock_guard<std::mutex> lock(mutex_);

    try {
        if (path == ":memory:" || path.empty()) {
            // Create in-memory database
            shared_db_ = std::make_unique<duckdb::DuckDB>(nullptr);
        } else {
            // Create file-based database
            shared_db_ = std::make_unique<duckdb::DuckDB>(path);
        }
        return true;
    } catch (const std::exception& e) {
        shared_db_ = nullptr;
        return false;
    }
}

duckdb::DuckDB* DuckDBInstanceManager::get_shared_db() {
    std::lock_guard<std::mutex> lock(mutex_);
    return shared_db_.get();
}

bool DuckDBInstanceManager::is_initialized() const {
    return shared_db_ != nullptr;
}

void DuckDBInstanceManager::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    shared_db_.reset();
}

} // namespace sql_practice
