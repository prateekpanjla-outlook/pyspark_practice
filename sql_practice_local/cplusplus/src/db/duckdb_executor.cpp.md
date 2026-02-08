# duckdb_executor.cpp - Documentation

## Purpose

Core database execution layer for the SQL Practice Server. Implements multi-instance DuckDB architecture for parallel query execution, schema initialization, and SQL validation.

## Architecture Overview

### Multi-Instance DuckDB Design

The server uses **8 separate DuckDB instances** to enable parallel query execution:

```
HTTP Request 1  HTTP Request 2  HTTP Request 3  ...  HTTP Request N
       |              |              |                   |
       v              v              v                   v
   Session 1      Session 2      Session 3  ...      Session N
       |              |              |                   |
       +--------------+--------------+-------------------+
                            |
                   Round-Robin Assignment
                            |
       +------+------+------+------+------+------+------+
       |      |      |      |      |      |      |      |
    Inst 0 Inst 1 Inst 2 Inst 3 Inst 4 Inst 5 Inst 6 Inst 7
```

**Benefits:**
- True parallelism: N queries execute simultaneously (one per instance)
- Eliminates single-threaded DuckDB bottleneck
- 5-8x throughput improvement for concurrent loads

**Memory Trade-off:**
- 8 instances: ~200 MB total (8 x 25 MB)
- Each session: ~1 KB for lightweight Connection object

## Key Classes

### DuckDBConnection

Wrapper around DuckDB connection that can operate in two modes:

#### Standalone Mode (Legacy)
```cpp
DuckDBConnection(const std::string& path);
```
Creates its own DuckDB instance. Used for testing or standalone operation.

#### Shared Mode (Production)
```cpp
DuckDBConnection(void* shared_db, size_t idx = 0);
```
Uses an existing DuckDB instance from the `DuckDBInstanceManager`. Tracks which instance it belongs to via `instance_index_` for telemetry.

**Key Members:**
```cpp
void* db;                // duckdb::DuckDB* (opaque pointer)
void* conn;              // duckdb::Connection* (opaque pointer)
bool owns_db;            // True if we should delete db on destruction
size_t instance_index_;  // Which instance this connection uses (for telemetry)
```

**Methods:**
- `execute(sql)`: Executes SQL and returns QueryResult with columns, rows, timing

### SQLExecutor

Main executor class that manages connections and schema initialization.

**Key Members:**
```cpp
std::string shared_db_path;  // Database path (typically ":memory:")
```

## Public Methods

### `create_connection()`
```cpp
std::unique_ptr<DuckDBConnection> SQLExecutor::create_connection();
```
Creates a new connection using round-robin assignment to distribute load across instances.

**Flow:**
1. Gets singleton `DuckDBInstanceManager`
2. Initializes if not already initialized
3. Calls `get_instance(&instance_idx)` for round-robin assignment
4. Returns `DuckDBConnection` with assigned instance index

### `initialize_schema()`
```cpp
bool SQLExecutor::initialize_schema(
    DuckDBConnection* conn,
    const QuestionSchema& schema
);
```
Initializes database schema for a single question.

**Steps:**
1. Creates tables using `CREATE TABLE IF NOT EXISTS`
2. Checks if table is empty (to avoid duplicate inserts)
3. Inserts sample data only if table is empty
4. Handles NULL values and string escaping

**Important:** Uses `IF NOT EXISTS` and row count checking to prevent duplicate data from concurrent initialization.

### `initialize_all_schemas()`
```cpp
static bool SQLExecutor::initialize_all_schemas(QuestionLoader* loader);
```
**Critical method** - initializes ALL question schemas on ALL DuckDB instances at server startup.

**Called from:** `main.cpp` before HTTP server starts.

**Process:**
1. Gets or creates 8 DuckDB instances via `DuckDBInstanceManager`
2. For each instance:
   - Creates a connection
   - Initializes schema for ALL questions
3. Prints progress for first instance only

**Why This Matters:**
- Prevents race condition where concurrent requests duplicate schema data
- Ensures all instances have identical schemas
- Single initialization point before any user requests

### `execute()`
```cpp
QueryResult SQLExecutor::execute(
    DuckDBConnection* conn,
    const std::string& sql
);
```
Executes a SQL query and returns results.

**Returns:**
- `success`: True if query succeeded
- `columns`: Column names
- `rows`: Result data as vector of column->value maps
- `execution_time_ms`: Query execution time
- `row_count`: Number of rows returned
- `error_message`: Error text if failed

### `is_safe_query()`
```cpp
bool SQLExecutor::is_safe_query(const std::string& sql) const;
```
Validates SQL for security. Blocks dangerous operations:

**Blocked Keywords:**
- DROP, DELETE, UPDATE, INSERT
- ALTER, TRUNCATE, CREATE, GRANT, REVOKE
- COPY, EXECUTE

**Additional Checks:**
- Only single statements allowed (one semicolon max)

## Result Chunk Processing

DuckDB returns results in chunks for efficiency. The executor iterates through chunks:

```cpp
while (true) {
    auto chunk = query_result->Fetch();
    if (!chunk || chunk->size() == 0) break;

    for (size_t row_idx = 0; row_idx < chunk->size(); ++row_idx) {
        // Extract each row
        for (size_t col_idx = 0; col_idx < columns; ++col_idx) {
            auto value = chunk->GetValue(col_idx, row_idx);
            // Convert to string and store
        }
    }
}
```

## Telemetry Integration

Each query execution records telemetry:

```cpp
// In DuckDBConnection::execute()
DuckDBInstanceManager::get().record_query(instance_index_);
```

This tracks:
- Per-instance query counts
- Connection distribution
- Round-robin overhead timing

## String Escaping

Sample data insertion handles string escaping:

```cpp
// Escape single quotes for SQL string literals
size_t pos = 0;
while ((pos = value.find("'", pos)) != std::string::npos) {
    value.replace(pos, 1, "''");
    pos += 2;
}
```

## Dependencies

| Header | Purpose |
|--------|---------|
| `include/sql_executor.hpp` | Main class definitions |
| `include/duckdb_instance_manager.hpp` | Multi-instance management |
| `include/question_loader.hpp` | Question schema definitions |
| `<duckdb.hpp>` | DuckDB database library |

## Related Files

- **sql_executor.hpp**: Header with class definitions
- **duckdb_instance_manager.cpp**: Manages multiple DuckDB instances
- **question_loader.cpp**: Provides question schemas

## Performance Characteristics

| Operation | Time Complexity | Notes |
|-----------|-----------------|-------|
| create_connection() | O(1) | Atomic round-robin assignment |
| initialize_schema() | O(n*m) | n tables, m rows per table |
| execute() | O(q) | Depends on query complexity |

## Known Issues

### Table Name Conflicts
Multiple questions use the same table name (e.g., `Employee`) with different schemas:
- q1, q3, q4, q5 all use `Employee`
- Last schema initialized overwrites previous ones
- Workaround: C++ load tester uses questions with unique tables

**TODO:** Implement table namespacing (e.g., `q1_Employee`, `q2_Person`)

## See Also

- [DuckDB Instance Manager](../core/duckdb_instance_manager.cpp.md)
- [Question Loader](question_loader.cpp.md)
- [Database Architecture](../DATABASE_ARCHITECTURE.md)
