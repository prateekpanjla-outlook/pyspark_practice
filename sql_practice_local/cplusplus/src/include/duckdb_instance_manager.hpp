#ifndef DUCKDB_INSTANCE_MANAGER_HPP
#define DUCKDB_INSTANCE_MANAGER_HPP

#include <duckdb.hpp>
#include <memory>
#include <mutex>

namespace sql_practice {

/**
 * @brief Manages shared DuckDB database instances
 *
 * Instead of each session creating its own DuckDB instance (~24MB each),
 * we create a single shared instance and all sessions get their own Connection to it.
 *
 * Memory savings:
 * - 1000 sessions with separate instances: ~24GB virtual memory
 * - 1000 sessions with shared instance: ~24MB virtual memory
 */
class DuckDBInstanceManager {
private:
    static std::unique_ptr<DuckDBInstanceManager> instance_;
    static std::mutex mutex_;

    // Shared DuckDB database instance
    std::unique_ptr<duckdb::DuckDB> shared_db_;

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
     * @brief Initialize the shared database instance
     * @param path Database path (":memory:" for in-memory)
     * @return true if initialization successful
     */
    bool initialize(const std::string& path = ":memory:");

    /**
     * @brief Get a pointer to the shared DuckDB instance
     * @return Pointer to duckdb::DuckDB or nullptr if not initialized
     */
    duckdb::DuckDB* get_shared_db();

    /**
     * @brief Check if the shared instance is initialized
     */
    bool is_initialized() const;

    /**
     * @brief Shutdown and cleanup the shared instance
     */
    void shutdown();
};

} // namespace sql_practice

#endif // DUCKDB_INSTANCE_MANAGER_HPP
