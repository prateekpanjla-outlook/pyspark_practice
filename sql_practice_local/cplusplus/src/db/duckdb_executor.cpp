#include "include/sql_executor.hpp"
#include <duckdb.hpp>
#include <chrono>
#include <sstream>
#include <algorithm>
#include <cctype>

namespace sql_practice {

// =============================================================================
// TODO: Shared DuckDB Instance Architecture
// =============================================================================
// Current Issue: Each session creates its own DuckDB instance (~24MB virtual memory each)
// With 1000 sessions = 24GB+ virtual memory (though actual RSS is much lower)
//
// Proposed Architecture:
// - Create N shared DuckDB instances (where N = total_sessions / connections_per_instance)
// - Each session gets a Connection to one of the shared instances
// - DuckDB can handle many concurrent connections per instance
//
// Example calculation:
// - Target: 10,000 concurrent users
// - Connections per DuckDB instance: ~100-1000 (DuckDB is quite efficient)
// - Number of instances needed: 10,000 / 500 = 20 instances
// - Memory per instance: ~24MB virtual
// - Total virtual memory: 20 * 24MB = 480MB (vs 240GB for separate instances)
//
// Implementation approach:
// 1. Create a connection pool that manages multiple DuckDB instances
// 2. Each session gets assigned a connection from the pool
// 3. Connections are returned to pool when session expires
// 4. Use round-robin or least-loaded assignment
// =============================================================================

// =============================================================================
// DuckDBConnection Implementation
// =============================================================================

DuckDBConnection::DuckDBConnection(const std::string& path) {
    try {
        // Create DuckDB instance (in-memory if path is ":memory:")
        if (path == ":memory:" || path.empty()) {
            db = nullptr;  // Will use default in-memory
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
    }
}

DuckDBConnection::~DuckDBConnection() {
    if (conn) {
        delete static_cast<duckdb::Connection*>(conn);
        conn = nullptr;
    }
    if (db) {
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
    return std::make_unique<DuckDBConnection>(":memory:");
}

bool SQLExecutor::initialize_schema(
    DuckDBConnection* conn,
    const QuestionSchema& schema
) {
    if (!conn) return false;

    try {
        // Create tables
        for (const auto& table : schema.tables) {
            std::stringstream sql;
            sql << "CREATE TABLE " << table.name << " (";

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

            // Insert sample data
            auto it = schema.sample_data.find(table.name);
            if (it != schema.sample_data.end()) {
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
