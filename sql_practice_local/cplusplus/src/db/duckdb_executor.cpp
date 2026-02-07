#include "include/sql_executor.hpp"
#include "include/duckdb_instance_manager.hpp"
#include "include/question_loader.hpp"
#include <duckdb.hpp>
#include <chrono>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <iostream>

namespace sql_practice {

// =============================================================================
// Shared DuckDB Instance Architecture (IMPLEMENTED)
// =============================================================================
//
// Solution: Single shared DuckDB instance for all sessions
// - DuckDBInstanceManager holds a single shared DuckDB instance
// - Each session creates its own Connection to the shared instance
// - DuckDB efficiently handles many concurrent connections
//
// Memory Savings:
// - Before: 1000 sessions × 24MB = ~24GB virtual memory
// - After:  1 shared instance (24MB) + 1000 connections (~1KB each) = ~25MB total
// - Improvement: ~1000x reduction in virtual memory usage
//
// Implementation:
// 1. DuckDBInstanceManager - Singleton that manages the shared database
// 2. DuckDBConnection - Two constructors: standalone and shared mode
// 3. SQLExecutor::create_connection() - Creates connections to shared DB
// 4. DuckDB connections are thread-safe for concurrent queries
//
// Future Enhancement:
// - For even larger scale (10K+ users), could implement connection pooling
// - with multiple shared instances based on CONNECTIONS_PER_INSTANCE
// =============================================================================

// =============================================================================
// DuckDBConnection Implementation
// =============================================================================

// Constructor for standalone mode (creates new DuckDB instance)
DuckDBConnection::DuckDBConnection(const std::string& path) : owns_db(true) {
    try {
        // Create DuckDB instance (in-memory if path is ":memory:")
        if (path == ":memory:" || path.empty()) {
            auto db_ptr = new duckdb::DuckDB(nullptr);
            db = static_cast<void*>(db_ptr);
            auto conn_ptr = new duckdb::Connection(*db_ptr);
            conn = static_cast<void*>(conn_ptr);
        } else {
            auto db_ptr = new duckdb::DuckDB(path);
            db = static_cast<void*>(db_ptr);
            auto conn_ptr = new duckdb::Connection(*db_ptr);
            conn = static_cast<void*>(conn_ptr);
        }
    } catch (const std::exception& e) {
        db = nullptr;
        conn = nullptr;
        owns_db = false;
    }
}

// Constructor for shared mode (uses existing DuckDB instance)
DuckDBConnection::DuckDBConnection(void* shared_db) : owns_db(false) {
    try {
        db = shared_db;  // Don't own this, won't delete it
        if (shared_db) {
            auto db_ptr = static_cast<duckdb::DuckDB*>(shared_db);
            auto conn_ptr = new duckdb::Connection(*db_ptr);
            conn = static_cast<void*>(conn_ptr);
        } else {
            conn = nullptr;
        }
    } catch (const std::exception& e) {
        db = nullptr;
        conn = nullptr;
    }
}

DuckDBConnection::~DuckDBConnection() {
    if (conn) {
        delete static_cast<duckdb::Connection*>(conn);
        conn = nullptr;
    }
    // Only delete db if we own it
    if (db && owns_db) {
        delete static_cast<duckdb::DuckDB*>(db);
        db = nullptr;
    }
}

QueryResult DuckDBConnection::execute(const std::string& sql) {
    QueryResult result;
    auto start = std::chrono::high_resolution_clock::now();

    try {
        auto* conn_ptr = static_cast<duckdb::Connection*>(conn);
        auto query_result = conn_ptr->Query(sql);

        // Check for errors
        if (query_result->HasError()) {
            result.success = false;
            result.error_message = query_result->GetError();
            result.execution_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::high_resolution_clock::now() - start
            ).count();
            return result;
        }

        result.success = true;

        // Get column names
        if (query_result->RowCount() > 0) {
            auto columns = query_result->ColumnCount();
            for (size_t i = 0; i < columns; ++i) {
                result.columns.push_back(query_result->ColumnName(i));
            }

            // Get rows - iterate through chunks using Fetch()
            size_t row_count = 0;
            while (true) {
                auto chunk = query_result->Fetch();
                if (!chunk || chunk->size() == 0) break;

                for (size_t row_idx = 0; row_idx < chunk->size(); ++row_idx) {
                    std::unordered_map<std::string, std::string> row_data;
                    for (size_t col_idx = 0; col_idx < columns; ++col_idx) {
                        std::string value_str;
                        auto value = chunk->GetValue(col_idx, row_idx);
                        if (value.IsNull()) {
                            value_str = "NULL";
                        } else {
                            value_str = value.ToString();
                        }
                        row_data[result.columns[col_idx]] = value_str;
                    }
                    result.rows.push_back(row_data);
                    row_count++;
                }
            }
            result.row_count = static_cast<int>(row_count);
        }

        result.execution_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now() - start
        ).count();

    } catch (const std::exception& e) {
        result.success = false;
        result.error_message = e.what();
        result.execution_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now() - start
        ).count();
    }

    return result;
}

// =============================================================================
// SQLExecutor Implementation
// =============================================================================

SQLExecutor::SQLExecutor(const std::string& db_path)
    : shared_db_path(db_path) {
}

std::unique_ptr<DuckDBConnection> SQLExecutor::create_connection() {
    // Get the shared database instance from the manager
    auto& manager = DuckDBInstanceManager::get();

    // Initialize if not already initialized
    if (!manager.is_initialized()) {
        manager.initialize(":memory:");
    }

    auto* shared_db = manager.get_shared_db();

    // Create a connection to the shared database
    return std::make_unique<DuckDBConnection>(shared_db);
}

bool SQLExecutor::initialize_schema(
    DuckDBConnection* conn,
    const QuestionSchema& schema
) {
    if (!conn) return false;

    try {
        // Create tables (use IF NOT EXISTS to handle concurrent initialization)
        for (const auto& table : schema.tables) {
            std::stringstream sql;
            sql << "CREATE TABLE IF NOT EXISTS " << table.name << " (";

            for (size_t i = 0; i < table.columns.size(); ++i) {
                const auto& col = table.columns[i];
                sql << col.name << " " << col.type;
                if (i < table.columns.size() - 1) {
                    sql << ", ";
                }
            }
            sql << ");";

            auto result = conn->execute(sql.str());
            if (!result.success) {
                return false;
            }

            // Insert sample data only if table is empty (to avoid duplicate inserts from concurrent requests)
            auto it = schema.sample_data.find(table.name);
            if (it != schema.sample_data.end()) {
                // Check if table already has data
                std::stringstream count_sql;
                count_sql << "SELECT COUNT(*) FROM " << table.name << ";";
                auto count_result = conn->execute(count_sql.str());

                // Only insert if table is empty (count = 0)
                if (count_result.success && count_result.rows.size() > 0) {
                    std::string count_str = count_result.rows[0].begin()->second;
                    int count = std::stoi(count_str);
                    if (count > 0) {
                        // Table already has data, skip insertion
                        continue;
                    }
                }

                for (const auto& row : it->second) {
                    std::stringstream insert_sql;
                    insert_sql << "INSERT INTO " << table.name << " VALUES (";

                    for (size_t i = 0; i < table.columns.size(); ++i) {
                        const auto& col = table.columns[i];
                        auto row_it = row.find(col.name);

                        if (row_it == row.end() || row_it->second == "NULL") {
                            insert_sql << "NULL";
                        } else if (col.type == "INTEGER" || col.type == "FLOAT") {
                            insert_sql << row_it->second;
                        } else {
                            // Escape string values
                            std::string value = row_it->second;
                            // Simple escape for single quotes
                            size_t pos = 0;
                            while ((pos = value.find("'", pos)) != std::string::npos) {
                                value.replace(pos, 1, "''");
                                pos += 2;
                            }
                            insert_sql << "'" << value << "'";
                        }

                        if (i < table.columns.size() - 1) {
                            insert_sql << ", ";
                        }
                    }
                    insert_sql << ");";

                    auto insert_result = conn->execute(insert_sql.str());
                    if (!insert_result.success) {
                        return false;
                    }
                }
            }
        }

        return true;

    } catch (const std::exception& e) {
        return false;
    }
}

// =============================================================================
// Global Schema Initialization (Called once at server startup)
// =============================================================================

bool SQLExecutor::initialize_all_schemas(QuestionLoader* loader) {
    try {
        // Get the shared database instance
        auto& manager = DuckDBInstanceManager::get();

        // Initialize the shared database if not already initialized
        if (!manager.is_initialized()) {
            if (!manager.initialize(":memory:")) {
                std::cerr << "Failed to initialize shared DuckDB instance" << std::endl;
                return false;
            }
        }

        // Create a connection for schema initialization
        auto* shared_db = manager.get_shared_db();
        if (!shared_db) {
            std::cerr << "Shared database instance is null" << std::endl;
            return false;
        }

        DuckDBConnection conn(shared_db);

        // Get all questions from the question loader
        auto questions = loader->list_questions();

        std::cout << "Initializing " << questions.size() << " question schemas..." << std::endl;

        // Initialize schema for each question
        for (const auto& question : questions) {
            SQLExecutor executor;
            bool initialized = executor.initialize_schema(&conn, question.schema);

            if (!initialized) {
                std::cerr << "Failed to initialize schema for question: " << question.id << std::endl;
                return false;
            }

            std::cout << "  ✓ Initialized schema for: " << question.title << " (" << question.id << ")" << std::endl;
        }

        std::cout << "All question schemas initialized successfully!" << std::endl;
        return true;

    } catch (const std::exception& e) {
        std::cerr << "Exception during schema initialization: " << e.what() << std::endl;
        return false;
    }
}

QueryResult SQLExecutor::execute(
    DuckDBConnection* conn,
    const std::string& sql
) {
    if (!conn) {
        QueryResult result;
        result.success = false;
        result.error_message = "Invalid database connection";
        return result;
    }

    return conn->execute(sql);
}

bool SQLExecutor::compare_results(
    const QueryResult& result,
    const std::vector<std::unordered_map<std::string, std::string>>& expected
) const {
    // Compare columns
    if (result.columns.size() != expected.empty() ? 0 : expected[0].size()) {
        return false;
    }

    // Compare row count
    if (result.rows.size() != expected.size()) {
        return false;
    }

    // Compare data
    for (size_t i = 0; i < result.rows.size(); ++i) {
        const auto& result_row = result.rows[i];
        const auto& expected_row = expected[i];

        for (const auto& col : result.columns) {
            auto result_val = result_row.find(col);
            auto expected_val = expected_row.find(col);

            if (result_val == result_row.end() || expected_val == expected_row.end()) {
                return false;
            }

            if (result_val->second != expected_val->second) {
                return false;
            }
        }
    }

    return true;
}

bool SQLExecutor::is_safe_query(const std::string& sql) const {
    // Convert to uppercase for checking
    std::string upper_sql = sql;
    std::transform(upper_sql.begin(), upper_sql.end(), upper_sql.begin(), ::toupper);

    // Blocked keywords
    const std::vector<std::string> blocked_keywords = {
        "DROP", "DELETE", "UPDATE", "INSERT",
        "ALTER", "TRUNCATE", "CREATE", "GRANT",
        "REVOKE", "COPY", "EXECUTE"
    };

    for (const auto& keyword : blocked_keywords) {
        if (upper_sql.find(keyword) != std::string::npos) {
            return false;
        }
    }

    // Check for multiple statements
    if (std::count(sql.begin(), sql.end(), ';') > 1) {
        return false;
    }

    return true;
}

} // namespace sql_practice
