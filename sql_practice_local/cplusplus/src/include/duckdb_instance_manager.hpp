#ifndef DUCKDB_INSTANCE_MANAGER_HPP
#define DUCKDB_INSTANCE_MANAGER_HPP

#include <duckdb.hpp>
#include <memory>
#include <mutex>
#include <vector>
#include <atomic>

namespace sql_practice {

/**
 * @brief Manages multiple DuckDB database instances for parallel query execution
 *
 * Architecture: Multiple separate DuckDB instances (each single-threaded)
 * - Each Oat++ worker thread gets its own DuckDB instance
 * - True parallelism: Multiple queries can execute simultaneously on different DB instances
 * - Trade-off: N× memory usage but eliminates query contention
 *
 * Memory:
 * - 8 instances: ~200MB (8 × 25MB)
 * - Each session has lightweight Connection object
 *
 * Performance benefit:
 * - Eliminates single-threaded DuckDB bottleneck
 * - N concurrent SQL queries can execute in parallel
 */
class DuckDBInstanceManager {
private:
    static std::unique_ptr<DuckDBInstanceManager> instance_;
    static std::mutex mutex_;

    // Multiple DuckDB instances for parallel execution
    std::vector<std::unique_ptr<duckdb::DuckDB>> db_instances_;

    // Number of instances to create (configurable)
    size_t num_instances_;

    // Round-robin counter for session assignment
    std::atomic<size_t> next_instance_index_{0};

    // Mutex for round-robin assignment
    std::atomic<size_t> assignment_lock_{0};

    // Private constructor for singleton
    DuckDBInstanceManager();

public:
    // Delete copy constructor and assignment operator
    DuckDBInstanceManager(const DuckDBInstanceManager&) = delete;
    DuckDBInstanceManager& operator=(const DuckDBInstanceManager&) = delete;

    /**
     * @brief Get the singleton instance
     */
    static DuckDBInstanceManager& get();

    /**
     * @brief Initialize all database instances
     * @param path Database path (":memory:" for in-memory)
     * @param num_instances Number of DuckDB instances to create (default: 8)
     * @return true if initialization successful
     */
    bool initialize(const std::string& path = ":memory:", size_t num_instances = 8);

    /**
     * @brief Get a DuckDB instance using round-robin assignment
     * @return Pointer to duckdb::DuckDB or nullptr if not initialized
     */
    duckdb::DuckDB* get_instance();

    /**
     * @brief Get a specific DuckDB instance by index
     * @param index Instance index (0 to num_instances-1)
     * @return Pointer to duckdb::DuckDB or nullptr if index invalid
     */
    duckdb::DuckDB* get_instance_by_index(size_t index);

    /**
     * @brief Get the shared instance (legacy method - returns first instance)
     * @deprecated Use get_instance() instead
     */
    duckdb::DuckDB* get_shared_db();

    /**
     * @brief Check if instances are initialized
     */
    bool is_initialized() const;

    /**
     * @brief Get number of instances
     */
    size_t get_instance_count() const { return db_instances_.size(); }

    /**
     * @brief Shutdown and cleanup all instances
     */
    void shutdown();
};

} // namespace sql_practice

#endif // DUCKDB_INSTANCE_MANAGER_HPP
