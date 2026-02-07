#include "include/duckdb_instance_manager.hpp"
#include <iostream>

namespace sql_practice {

// Static member initialization
std::unique_ptr<DuckDBInstanceManager> DuckDBInstanceManager::instance_ = nullptr;
std::mutex DuckDBInstanceManager::mutex_;

DuckDBInstanceManager::DuckDBInstanceManager() : num_instances_(8) {
    // Private constructor
}

DuckDBInstanceManager& DuckDBInstanceManager::get() {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!instance_) {
        instance_ = std::unique_ptr<DuckDBInstanceManager>(new DuckDBInstanceManager());
    }

    return *instance_;
}

bool DuckDBInstanceManager::initialize(const std::string& path, size_t num_instances) {
    std::lock_guard<std::mutex> lock(mutex_);

    try {
        num_instances_ = num_instances;
        db_instances_.clear();
        db_instances_.reserve(num_instances_);

        std::cout << "   📊 Creating " << num_instances << " DuckDB instances..." << std::endl;

        for (size_t i = 0; i < num_instances_; ++i) {
            std::unique_ptr<duckdb::DuckDB> db;

            if (path == ":memory:" || path.empty()) {
                db = std::make_unique<duckdb::DuckDB>(nullptr);
            } else {
                db = std::make_unique<duckdb::DuckDB>(path.c_str());
            }

            if (!db) {
                std::cerr << "   ❌ Failed to create DuckDB instance " << i << std::endl;
                db_instances_.clear();
                return false;
            }

            db_instances_.push_back(std::move(db));
        }

        std::cout << "   ✅ Created " << db_instances_.size() << " DuckDB instances"
                  << " (parallel query execution enabled)" << std::endl;
        return true;

    } catch (const std::exception& e) {
        std::cerr << "   ❌ Failed to initialize DuckDB instances: " << e.what() << std::endl;
        db_instances_.clear();
        return false;
    }
}

duckdb::DuckDB* DuckDBInstanceManager::get_instance() {
    // Round-robin assignment
    size_t index = next_instance_index_.fetch_add(1, std::memory_order_relaxed) % db_instances_.size();
    return db_instances_[index].get();
}

duckdb::DuckDB* DuckDBInstanceManager::get_shared_db() {
    // Legacy method - returns first instance
    if (db_instances_.empty()) {
        return nullptr;
    }
    return db_instances_[0].get();
}

bool DuckDBInstanceManager::is_initialized() const {
    return !db_instances_.empty();
}

void DuckDBInstanceManager::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    std::cout << "   🧹 Shutting down " << db_instances_.size() << " DuckDB instances..." << std::endl;
    db_instances_.clear();
}

} // namespace sql_practice
