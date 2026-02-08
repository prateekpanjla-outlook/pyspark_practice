# Database Layer Testing

## Overview

This document describes the testing approach for the database layer components in the SQL Practice Server.

## Components

The database layer consists of the following components:

| Component | Files | Purpose |
|-----------|-------|---------|
| Embedded Questions | `embedded_questions.hpp`, `embedded_questions.cpp` | Hardcoded question data (10 SQL questions) |
| Question Loader | `question_loader.cpp`, `question_loader.hpp` | Loads and manages questions at runtime |
| SQL Executor | `duckdb_executor.cpp`, `sql_executor.hpp` | Executes queries and manages schemas |
| Instance Manager | `duckdb_instance_manager.hpp`, `core/duckdb_instance_manager.cpp` | Manages 8 DuckDB instances for parallel execution |

## Test Coverage Areas

### Unit Tests

| Component | Test Coverage | Status |
|-----------|---------------|--------|
| Embedded Questions | Question data structure validation | Pending |
| Question Loader | Load, filter, and query operations | Pending |
| DuckDB Connection | Standalone and shared modes | Pending |
| SQL Executor | Schema initialization, query execution | Pending |
| Instance Manager | Round-robin assignment, telemetry | Verified via load testing |

### Integration Tests

| Scenario | Description | Status |
|----------|-------------|--------|
| Schema Init | All schemas initialize correctly on all instances | Manual |
| Query Execution | Concurrent queries execute in parallel | Load tested (1000 concurrent) |
| Telemetry | Query tracking reports accurately | Verified |
| Auto-Shutdown | Server shuts down after timeout | Implemented |

## Running Tests

### Manual Testing

```bash
# From the build VM
cd /home/vagrant/project/cplusplus/build

# Run the server with verbose output
./sql-practice-server

# Test with curl
curl -X POST http://localhost:9000/api/execute \
  -H "Content-Type: application/json" \
  -d '{"question_slug": "second-highest-salary", "query": "SELECT MAX(salary) AS SecondHighestSalary FROM Employee WHERE salary < (SELECT MAX(salary) FROM Employee)"}'
```

### Load Testing

See the load testing tools in `load_tester/`:
- C++ Load Tester: `load_tester/src/`
- Monitoring: `load_tester/server_monitor.sh`
- Percentile test: `load_tester/percentile_test.sh`
- Incremental test: `load_tester/incremental_test.sh`

### Performance Benchmarks

| Concurrent Users | Requests | Accuracy | Avg Response Time | 95th %ile |
|------------------|----------|----------|-------------------|-----------|
| 25 | 50 | 100% | ~400 ms | ~600 ms |
| 50 | 100 | 100% | ~800 ms | ~1200 ms |
| 100 | 200 | 100% | ~1700 ms | ~4500 ms |
| 1000 | 2000 | 100% | 4243 ms | ~8000 ms |

## Known Issues

### Table Name Conflicts

Multiple questions use the same table name (e.g., `Employee`) with different schemas. The last-initialized schema overwrites previous ones.

**Workaround:** C++ load tester uses questions with unique tables:
- q2: Person (unique)
- q8: Logs (unique)
- q9: Orders (unique)

**TODO:** Implement table namespacing (e.g., `q1_Employee`, `q2_Person`) or separate database per question.

### Single-Threaded DuckDB Bottleneck (Mitigated)

The original architecture used a single DuckDB instance, causing all queries to execute sequentially. This has been **resolved** by implementing 8 separate DuckDB instances with round-robin session assignment.

**Before:** 1 DuckDB instance, all queries serialized
**After:** 8 DuckDB instances, up to 8 queries in parallel

## Documentation Files

| File | Description |
|------|-------------|
| [embedded_questions.hpp.md](embedded_questions.hpp.md) | Header file with data structure definitions |
| [embedded_questions.cpp.md](embedded_questions.cpp.md) | Implementation with 10 hardcoded questions |
| [question_loader.cpp.md](question_loader.cpp.md) | Loads and manages questions at runtime |
| [duckdb_executor.cpp.md](duckdb_executor.cpp.md) | Core database execution layer |
| [../include/sql_executor.hpp.md](../include/sql_executor.hpp.md) | SQL executor and connection interfaces |
| [../include/question_loader.hpp.md](../include/question_loader.hpp.md) | Question loader interface |
| [../core/duckdb_instance_manager.cpp.md](../core/duckdb_instance_manager.cpp.md) | Multi-instance management |

## Related Documentation

- [Database Architecture](../DATABASE_ARCHITECTURE.md)
- [DuckDB Executor](duckdb_executor.md)
- [Session Manager](../core/session_manager.md)
