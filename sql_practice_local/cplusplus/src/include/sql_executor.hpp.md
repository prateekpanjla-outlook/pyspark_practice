# sql_executor.hpp - Documentation

## Purpose

Header file defining the core database execution interfaces. Contains `QueryResult`, `QuestionSchema`, `SQLExecutor`, and `DuckDBConnection` class declarations.

## Key Structures

### QueryResult
```cpp
struct QueryResult {
    bool success;                                  // Query execution success
    std::string error_message;                     // Error text if failed

    // Output data
    std::vector<std::string> columns;             // Column names
    std::vector<std::unordered_map<std::string, std::string>> rows;  // Result rows

    // Execution metrics
    int64_t execution_time_ms;                     // Query execution time
    int row_count;                                 // Number of rows returned

    // Comparison with expected output
    bool is_correct;                               // True if matches expected
};
```
Represents the result of a SQL query execution. Contains both the output data and metadata about the execution.

### QuestionSchema
```cpp
struct QuestionSchema {
    struct Column {
        std::string name;      // Column name
        std::string type;      // SQL type (INTEGER, VARCHAR, etc.)
    };

    struct Table {
        std::string name;                      // Table name
        std::vector<Column> columns;           // Table columns
    };

    std::vector<Table> tables;                  // All tables in schema
    std::unordered_map<std::string, std::vector<std::unordered_map<std::string, std::string>>> sample_data;
};
```
Defines the database schema for a question, including table structures and sample data.

## Key Classes

### SQLExecutor

Main executor class for database operations.

```cpp
class SQLExecutor {
private:
    std::string shared_db_path;

public:
    explicit SQLExecutor(const std::string& db_path = ":memory:");

    std::unique_ptr<DuckDBConnection> create_connection();
    bool initialize_schema(DuckDBConnection* conn, const QuestionSchema& schema);
    static bool initialize_all_schemas(QuestionLoader* loader);
    QueryResult execute(DuckDBConnection* conn, const std::string& sql);
    bool compare_results(const QueryResult& result,
                         const std::vector<std::unordered_map<std::string, std::string>>& expected) const;
    bool is_safe_query(const std::string& sql) const;
};
```

**Key Methods:**

| Method | Purpose |
|--------|---------|
| `create_connection()` | Creates new connection with round-robin assignment |
| `initialize_schema()` | Initializes tables and sample data for a question |
| `initialize_all_schemas()` | Static method to init ALL schemas on startup |
| `execute()` | Executes SQL query and returns results |
| `compare_results()` | Compares query output with expected results |
| `is_safe_query()` | Validates SQL for dangerous keywords |

### DuckDBConnection

Wrapper for DuckDB database connections.

```cpp
class DuckDBConnection {
private:
    void* db;                  // duckdb::DuckDB* (opaque pointer)
    void* conn;                // duckdb::Connection* (opaque pointer)
    bool owns_db;              // Ownership flag for db
    size_t instance_index_;    // Which DuckDB instance (for telemetry)

public:
    // Standalone mode - creates own instance
    DuckDBConnection(const std::string& path);

    // Shared mode - uses existing instance
    DuckDBConnection(void* shared_db, size_t idx = 0);

    ~DuckDBConnection();

    QueryResult execute(const std::string& sql);
    void* get_connection() const;
    size_t get_instance_index() const;
};
```

**Dual Mode Operation:**

| Mode | Constructor | Ownership | Use Case |
|------|-------------|-----------|----------|
| Standalone | `DuckDBConnection(path)` | Owns db | Testing, standalone |
| Shared | `DuckDBConnection(shared_db, idx)` | Borrows db | Production server |

## Architecture Patterns

### Opaque Pointers
DuckDB types are stored as `void*` to avoid including DuckDB headers in this header:
```cpp
void* db;    // Actually duckdb::DuckDB*
void* conn;  // Actually duckdb::Connection*
```
This reduces compilation dependencies and speeds up build times.

### Static Factory Method
`initialize_all_schemas()` is static because it initializes the global `DuckDBInstanceManager` singleton before any connections are created.

## Dependencies

| Header | Purpose |
|--------|---------|
| `<string>` | String types |
| `<vector>` | Dynamic arrays |
| `<unordered_map>` | Hash maps |
| `<memory>` | Smart pointers (unique_ptr) |

## Related Files

- **sql_executor.cpp**: Implementation of executor
- **duckdb_instance_manager.hpp**: Multi-instance management
- **question_loader.hpp**: Question definitions

## Forward Declarations

```cpp
class DuckDBConnection;  // Forward declared in SQLExecutor
class QuestionLoader;    // Forward declared in initialize_all_schemas()
```
These are forward declared to avoid circular dependencies.
