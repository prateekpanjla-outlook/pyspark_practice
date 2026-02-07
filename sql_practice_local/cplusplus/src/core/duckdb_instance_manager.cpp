#include "include/duckdb_instance_manager.hpp"
#include <iostream>
#include <chrono>
#include <iomanip>

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

        // Initialize telemetry counters
        {
            std::lock_guard<std::mutex> lock(telemetry_mutex_);
            connection_counts_.assign(num_instances_, 0);
            query_counts_.assign(num_instances_, 0);
            total_get_instance_calls_ = 0;
            total_get_instance_ns_ = 0;
        }

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

duckdb::DuckDB* DuckDBInstanceManager::get_instance(size_t* out_index) {
    // Track timing for telemetry
    auto start = std::chrono::high_resolution_clock::now();

    // Round-robin assignment
    size_t index = next_instance_index_.fetch_add(1, std::memory_order_relaxed) % db_instances_.size();

    // Update telemetry (with mutex to protect the regular variables)
    {
        std::lock_guard<std::mutex> lock(telemetry_mutex_);
        connection_counts_[index]++;
        total_get_instance_calls_++;
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

    {
        std::lock_guard<std::mutex> lock(telemetry_mutex_);
        total_get_instance_ns_ += ns;
    }

    // Output the instance index if requested
    if (out_index) {
        *out_index = index;
    }

    return db_instances_[index].get();
}

duckdb::DuckDB* DuckDBInstanceManager::get_instance_by_index(size_t index) {
    if (index >= db_instances_.size()) {
        return nullptr;
    }
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

    // Print telemetry before shutdown
    print_telemetry();

    db_instances_.clear();
}

void DuckDBInstanceManager::record_query(size_t instance_index) {
    std::lock_guard<std::mutex> lock(telemetry_mutex_);
    if (instance_index < query_counts_.size()) {
        query_counts_[instance_index]++;
    }
}

void DuckDBInstanceManager::print_telemetry() {
    std::vector<size_t> conns, queries;
    size_t total_calls = 0;
    size_t total_ns = 0;

    // Copy telemetry data under lock
    {
        std::lock_guard<std::mutex> lock(telemetry_mutex_);
        conns = connection_counts_;
        queries = query_counts_;
        total_calls = total_get_instance_calls_;
        total_ns = total_get_instance_ns_;
    }

    std::cout << "\n╔═══════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║         DuckDB Instance Telemetry                     ║" << std::endl;
    std::cout << "╚═══════════════════════════════════════════════════════╝" << std::endl;

    size_t total_connections = 0;
    size_t total_queries = 0;

    for (size_t i = 0; i < db_instances_.size(); ++i) {
        total_connections += conns[i];
        total_queries += queries[i];

        std::cout << "Instance " << i << ": "
                  << conns[i] << " connections, "
                  << queries[i] << " queries";

        if (conns[i] > 0) {
            double avg_queries = static_cast<double>(queries[i]) / conns[i];
            std::cout << ", avg " << std::fixed << std::setprecision(1) << avg_queries << " queries/connection";
        }
        std::cout << std::endl;
    }

    std::cout << "\nTotal: " << total_connections << " connections, "
              << total_queries << " queries" << std::endl;

    if (total_calls > 0) {
        double avg_ns = static_cast<double>(total_ns) / total_calls;
        std::cout << "Round-robin overhead: " << total_calls << " calls, "
                  << std::fixed << std::setprecision(2) << avg_ns << " ns avg ("
                  << std::setprecision(4) << (avg_ns / 1000000.0) << " ms)" << std::endl;
    }

    std::cout << std::endl;

    // CRITICAL: Flush output so it appears in log file immediately
    std::cout << std::flush;
}

} // namespace sql_practice
