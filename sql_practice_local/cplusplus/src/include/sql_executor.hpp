#ifndef SQL_EXECUTOR_HPP
#define SQL_EXECUTOR_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

namespace sql_practice {

/**
 * @brief Result of SQL query execution
 */
struct QueryResult {
    bool success;
    std::string error_message;

    // Output data
    std::vector<std::string> columns;
    std::vector<std::unordered_map<std::string, std::string>> rows;

    // Execution metrics
    int64_t execution_time_ms;
    int row_count;

    // Comparison with expected output
    bool is_correct;

    QueryResult()
        : success(false), execution_time_ms(0), row_count(0), is_correct(false) {}
};

/**
 * @brief Question schema and data
 */
struct QuestionSchema {
    struct Column {
        std::string name;
        std::string type;  // INTEGER, VARCHAR, etc.
    };

    struct Table {
        std::string name;
        std::vector<Column> columns;
    };

    std::vector<Table> tables;
    std::unordered_map<std::string, std::vector<std::unordered_map<std::string, std::string>>> sample_data;
};

/**
 * @brief SQL Executor using DuckDB
 *
 * Executes SQL queries using shared database instance
 * All sessions share a single DuckDB instance but have isolated connections
 */
class SQLExecutor {
private:
    std::string shared_db_path;

public:
    explicit SQLExecutor(const std::string& db_path = ":memory:");

    /**
     * @brief Create a new connection to the shared database
     *
     * @return Connection to the shared DuckDB instance
     */
    std::unique_ptr<class DuckDBConnection> create_connection();

    /**
     * @brief Initialize database schema for a question
     */
    bool initialize_schema(
        DuckDBConnection* conn,
        const QuestionSchema& schema
    );

    /**
     * @brief Initialize all question schemas once at startup
     * Call this before accepting any connections
     */
    static bool initialize_all_schemas(class QuestionLoader* loader);

    /**
     * @brief Execute SQL query
     */
    QueryResult execute(
        DuckDBConnection* conn,
        const std::string& sql
    );

    /**
     * @brief Compare result with expected output
     */
    bool compare_results(
        const QueryResult& result,
        const std::vector<std::unordered_map<std::string, std::string>>& expected
    ) const;

    /**
     * @brief Validate SQL safety (check for dangerous keywords)
     */
    bool is_safe_query(const std::string& sql) const;
};

/**
 * @brief DuckDB connection wrapper
 *
 * Can operate in two modes:
 * 1. Standalone: Creates its own DuckDB instance (original behavior)
 * 2. Shared: Uses a shared DuckDB instance from DuckDBInstanceManager
 */
class DuckDBConnection {
private:
    void* db;      // duckdb::Database (owned if standalone, null if shared)
    void* conn;    // duckdb::Connection
    bool owns_db;  // True if we own the db instance and should delete it
    size_t instance_index_;  // Which DuckDB instance this connection belongs to (for telemetry)

public:
    // Constructor for standalone mode (creates new DuckDB instance)
    DuckDBConnection(const std::string& path);

    // Constructor for shared mode (uses existing DuckDB instance)
    DuckDBConnection(void* shared_db, size_t idx = 0);

    ~DuckDBConnection();

    QueryResult execute(const std::string& sql);
    void* get_connection() const { return conn; }
    size_t get_instance_index() const { return instance_index_; }
};

} // namespace sql_practice

#endif // SQL_EXECUTOR_HPP
