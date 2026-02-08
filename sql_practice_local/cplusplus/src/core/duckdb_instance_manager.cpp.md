# duckdb_instance_manager.cpp - Documentation

## Purpose

Manages multiple DuckDB database instances for parallel query execution. Implements a singleton pattern with round-robin session assignment to enable true parallel SQL execution.

## Architecture

### Multi-Instance Design

```
DuckDBInstanceManager (Singleton)
    |
    +-- db_instances_[0]  --> DuckDB Instance 0
    +-- db_instances_[1]  --> DuckDB Instance 1
    +-- db_instances_[2]  --> DuckDB Instance 2
    +-- ...
    +-- db_instances_[7]  --> DuckDB Instance 7

Round-Robin Assignment:
    Session 1 -> Instance 0
    Session 2 -> Instance 1
    Session 3 -> Instance 2
    ...
    Session 9 -> Instance 0 (wraps around)
```

### Benefits

| Aspect | Single Instance | Multi-Instance (8) |
|--------|----------------|-------------------|
| Parallel Queries | 1 (serialized) | 8 (simultaneous) |
| Memory | ~25 MB | ~200 MB |
| Throughput | ~60 req/s | ~170+ req/s |
| Bottleneck | Single-threaded | Distributed |

## Key Class: DuckDBInstanceManager

### Singleton Pattern
```cpp
class DuckDBInstanceManager {
private:
    static std::unique_ptr<DuckDBInstanceManager> instance_;
    static std::mutex mutex_;

    DuckDBInstanceManager();  // Private constructor

public:
    static DuckDBInstanceManager& get();
    // Deleted copy operations
    DuckDBInstanceManager(const DuckDBInstanceManager&) = delete;
    DuckDBInstanceManager& operator=(const DuckDBInstanceManager&) = delete;
};
```

### Data Members

```cpp
// Multiple DuckDB instances
std::vector<std::unique_ptr<duckdb::DuckDB>> db_instances_;

// Configuration
size_t num_instances_;  // Default: 8

// Round-robin assignment
std::atomic<size_t> next_instance_index_{0};

// Telemetry (protected by telemetry_mutex_)
std::vector<size_t> connection_counts_;
std::vector<size_t> query_counts_;
size_t total_get_instance_calls_;
size_t total_get_instance_ns_;
std::mutex telemetry_mutex_;
```

## Public Methods

### `get()`
```cpp
static DuckDBInstanceManager& get();
```
Returns the singleton instance. Thread-safe via double-checked locking pattern.

### `initialize()`
```cpp
bool initialize(const std::string& path = ":memory:", size_t num_instances = 8);
```
Initializes all database instances.

**Process:**
1. Clears existing instances
2. Creates N new DuckDB instances (in-memory by default)
3. Initializes telemetry counters
4. Returns true on success

**Example:**
```cpp
auto& manager = DuckDBInstanceManager::get();
manager.initialize(":memory:", 8);  // 8 in-memory instances
```

### `get_instance()`
```cpp
duckdb::DuckDB* get_instance(size_t* out_index = nullptr);
```
Returns a DuckDB instance using **round-robin assignment**.

**Algorithm:**
```cpp
size_t index = next_instance_index_.fetch_add(1) % db_instances_.size();
connection_counts_[index]++;
if (out_index) *out_index = index;
return db_instances_[index].get();
```

**Performance:** Atomic fetch_add is ~460 nanoseconds (negligible overhead).

### `get_instance_by_index()`
```cpp
duckdb::DuckDB* get_instance_by_index(size_t index);
```
Returns a specific instance by index. Used during schema initialization to ensure all instances are initialized identically.

### `get_shared_db()`
```cpp
duckdb::DuckDB* get_shared_db();
```
Legacy method - returns the first instance. **Deprecated** in favor of `get_instance()`.

### `is_initialized()`
```cpp
bool is_initialized() const;
```
Returns true if at least one instance exists.

### `get_instance_count()`
```cpp
size_t get_instance_count() const;
```
Returns the number of instances (typically 8).

### `shutdown()`
```cpp
void shutdown();
```
Prints telemetry and clears all instances. Called during graceful shutdown.

## Telemetry

### `record_query()`
```cpp
void record_query(size_t instance_index);
```
Records that a query was executed on a specific instance. Called from `DuckDBConnection::execute()`.

### `print_telemetry()`
```cpp
void print_telemetry();
```
Prints detailed statistics:

```
╔═══════════════════════════════════════════════════════╗
║         DuckDB Instance Telemetry                     ║
╚═══════════════════════════════════════════════════════╝
Instance 0: 2 connections, 442 queries, avg 221.0 queries/connection
Instance 1: 2 connections, 2 queries, avg 1.0 queries/connection
Instance 2: 1 connections, 1 queries, avg 1.0 queries/connection
...
Instance 7: 1 connections, 1 queries, avg 1.0 queries/connection

Total: 10 connections, 450 queries
Round-robin overhead: 10 calls, 461.00 ns avg (0.0005 ms)
```

**Metrics Tracked:**
- Per-instance connection count
- Per-instance query count
- Average queries per connection
- Round-robin assignment overhead

## Thread Safety

| Component | Protection |
|-----------|------------|
| `instance_` | `mutex_` (singleton initialization) |
| `db_instances_` | `mutex_` (initialize/shutdown) |
| `next_instance_index_` | `std::atomic` (lock-free) |
| Telemetry data | `telemetry_mutex_` |

## Memory Usage

| Component | Memory |
|-----------|--------|
| Base DuckDB instance | ~25 MB |
| 8 instances total | ~200 MB |
| Per-session connection | ~1 KB |

## Dependencies

| Header | Purpose |
|--------|---------|
| `duckdb.hpp` | DuckDB database library |
| `<mutex>` | std::mutex, lock_guard |
| `<atomic>` | std::atomic for round-robin |
| `<chrono>` | Telemetry timing |

## Related Files

- **duckdb_instance_manager.hpp**: Header with class definition
- **duckdb_executor.cpp**: Uses the instance manager
- **sql_executor.hpp**: DuckDBConnection wrapper

## Usage Flow

```
Server Startup
       |
       v
DuckDBInstanceManager::get().initialize(":memory:", 8)
       |
       v
SQLExecutor::initialize_all_schemas(loader)
       | (calls get_instance_by_index for each instance)
       v
HTTP Request -> create_connection()
       |
       v
get_instance(&instance_idx)  // Round-robin assignment
       |
       v
DuckDBConnection(db_instance, instance_idx)
       |
       v
Query executes -> record_query(instance_idx)
```

## Performance Characteristics

| Operation | Time | Notes |
|-----------|------|-------|
| initialize() | O(n) | n = num_instances |
| get_instance() | O(1) | Atomic fetch_add |
| record_query() | O(1) | With mutex lock |
| print_telemetry() | O(n) | n = num_instances |

## Known Issues

None at this time. The multi-instance architecture successfully resolves the single-threaded DuckDB bottleneck.

## See Also

- [DuckDB Executor](../db/duckdb_executor.cpp.md)
- [SQL Executor Header](../include/sql_executor.hpp.md)
- [Instance Manager Header](../include/duckdb_instance_manager.hpp.md)
