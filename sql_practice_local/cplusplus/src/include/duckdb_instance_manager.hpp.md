# duckdb_instance_manager.hpp - Documentation

## Purpose

Header file defining the `DuckDBInstanceManager` class - a singleton that manages multiple DuckDB database instances for parallel query execution.

## Class Overview

```cpp
class DuckDBInstanceManager {
private:
    static std::unique_ptr<DuckDBInstanceManager> instance_;
    static std::mutex mutex_;

    std::vector<std::unique_ptr<duckdb::DuckDB>> db_instances_;
    size_t num_instances_;
    std::atomic<size_t> next_instance_index_{0};

    std::vector<size_t> connection_counts_;
    std::vector<size_t> query_counts_;
    size_t total_get_instance_calls_{0};
    size_t total_get_instance_ns_{0};
    std::mutex telemetry_mutex_;

    DuckDBInstanceManager();  // Private constructor

public:
    // Singleton access
    static DuckDBInstanceManager& get();

    // Lifecycle
    bool initialize(const std::string& path = ":memory:", size_t num_instances = 8);
    void shutdown();
    bool is_initialized() const;

    // Instance access
    duckdb::DuckDB* get_instance(size_t* out_index = nullptr);
    duckdb::DuckDB* get_instance_by_index(size_t index);
    duckdb::DuckDB* get_shared_db();  // Deprecated
    size_t get_instance_count() const;

    // Telemetry
    void print_telemetry();
    void record_query(size_t instance_index);

    // Deleted for singleton
    DuckDBInstanceManager(const DuckDBInstanceManager&) = delete;
    DuckDBInstanceManager& operator=(const DuckDBInstanceManager&) = delete;
};
```

## Design Patterns

### Singleton Pattern
```cpp
static DuckDBInstanceManager& get();
```
Ensures only one instance manager exists throughout the application lifetime.

**Thread-Safe Implementation:**
```cpp
DuckDBInstanceManager& DuckDBInstanceManager::get() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!instance_) {
        instance_ = std::unique_ptr<DuckDBInstanceManager>(
            new DuckDBInstanceManager()
        );
    }
    return *instance_;
}
```

### Round-Robin Load Distribution
```cpp
std::atomic<size_t> next_instance_index_{0};
```
Atomic counter ensures thread-safe, lock-free round-robin assignment.

## Public Methods

### Lifecycle

| Method | Description |
|--------|-------------|
| `initialize(path, num_instances)` | Creates N DuckDB instances (default: 8) |
| `shutdown()` | Prints telemetry and clears instances |
| `is_initialized()` | Returns true if instances exist |

### Instance Access

| Method | Description | Thread Safety |
|--------|-------------|---------------|
| `get_instance(out_index)` | Round-robin assignment | Lock-free atomic |
| `get_instance_by_index(index)` | Direct index access | Thread-safe |
| `get_shared_db()` | Legacy - returns instance 0 | Thread-safe |
| `get_instance_count()` | Returns number of instances | Thread-safe |

### Telemetry

| Method | Description |
|--------|-------------|
| `record_query(instance_index)` | Records query execution for telemetry |
| `print_telemetry()` | Prints statistics to stdout |

## Memory Layout

```
DuckDBInstanceManager
    |
    +-- db_instances_: vector<unique_ptr<DuckDB>>
    |       |
    |       +-- [0]: DuckDB Instance (~25 MB)
    |       +-- [1]: DuckDB Instance (~25 MB)
    |       +-- ...
    |       +-- [7]: DuckDB Instance (~25 MB)
    |
    +-- next_instance_index_: atomic<size_t> (8 bytes)
    |
    +-- connection_counts_: vector<size_t> (8 * 8 = 64 bytes)
    +-- query_counts_: vector<size_t> (8 * 8 = 64 bytes)
    +-- total_get_instance_calls_: size_t (8 bytes)
    +-- total_get_instance_ns_: size_t (8 bytes)
```

**Total Memory:** ~200 MB (mostly from DuckDB instances)

## Thread Safety Guarantees

| Member | Protection | Access Pattern |
|--------|------------|----------------|
| `instance_` | `mutex_` | Singleton initialization |
| `db_instances_` | `mutex_` | Initialize/shutdown only |
| `next_instance_index_` | `std::atomic` | Lock-free read-modify-write |
| Telemetry fields | `telemetry_mutex_` | Protected by mutex |

## Dependencies

| Header | Purpose |
|--------|---------|
| `<duckdb.hpp>` | DuckDB database library |
| `<memory>` | std::unique_ptr |
| `<mutex>` | std::mutex |
| `<atomic>` | std::atomic |
| `<vector>` | std::vector |

## Related Files

- **duckdb_instance_manager.cpp**: Implementation
- **duckdb_executor.cpp**: Uses instance manager for connections
- **sql_executor.hpp**: DuckDBConnection wrapper

## Example Usage

```cpp
#include "include/duckdb_instance_manager.hpp"

using namespace sql_practice;

// Initialize at startup
auto& manager = DuckDBInstanceManager::get();
manager.initialize(":memory:", 8);

// Get instance with round-robin assignment
size_t instance_idx;
duckdb::DuckDB* db = manager.get_instance(&instance_idx);

// Create connection to assigned instance
duckdb::Connection conn(*db);

// Execute query...
auto result = conn.Query("SELECT * FROM Employee");

// Record for telemetry
manager.record_query(instance_idx);

// Shutdown (prints telemetry)
manager.shutdown();
```

## Configuration

Default settings (configurable via `initialize()`):
- **Number of instances:** 8
- **Database path:** ":memory:" (in-memory)
- **Round-robin:** Enabled via atomic counter

## See Also

- [Instance Manager Implementation](../core/duckdb_instance_manager.cpp.md)
- [DuckDB Executor](../db/duckdb_executor.cpp.md)
