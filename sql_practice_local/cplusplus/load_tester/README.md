# SQL Practice Server - Load Testing Tool

## Overview

This tool simulates concurrent users to stress test the SQL Practice Server. Each user makes 2 attempts per question:
1. **First attempt**: Incorrect SQL solution (should be graded as wrong)
2. **Second attempt**: Correct SQL solution (should be graded as correct)

## Building

### Linux/Ubuntu (libcurl required)

```bash
cd load_tester
mkdir build && cd build
cmake ..
make -j8
```

### Installing libcurl

```bash
sudo apt-get install libcurl4-openssl-dev
```

## Usage

### Basic Usage (1000 users, localhost)

```bash
./load-tester
```

### Custom Server URL

```bash
./load-tester http://192.168.1.100:8080
```

### Custom Number of Users

```bash
./load-tester http://localhost:8080 500
```

## Output Example

```
╔════════════════════════════════════════════════════════╗
║     SQL Practice Server - Load Testing Tool            ║
╚════════════════════════════════════════════════════════╝

Configuration:
  Server URL: http://localhost:8080
  Concurrent Users: 1000
  Attempts per User: 2 (wrong answer, then correct)
  Total Requests: 2000

Creating 1000 user sessions...
  Created 100 sessions...
  Created 200 sessions...
  ...
  Created 1000 sessions...
✅ All 1000 sessions created successfully

🚀 Starting load test with 1000 concurrent users...
Server: http://localhost:8080
Each user will run 2 attempts (wrong answer, then correct answer)

User 0 [Second Highest Salary]: Attempt 1: ❌ (15 ms), Attempt 2: ✅ (12 ms)
User 1 [Duplicate Emails]: Attempt 1: ❌ (18 ms), Attempt 2: ✅ (14 ms)
...

⏱️  Total test duration: 1250 ms
📊 Throughput: 1600.00 requests/second

╔════════════════════════════════════════════════════════╗
║           LOAD TEST RESULTS                              ║
╠════════════════════════════════════════════════════════╣
║ Total Attempts:     2000
║ Successful:         2000
║ Failed:             0
║ Correct Answers:    1000
║ Wrong Answers:      1000
╠════════════════════════════════════════════════════════╣
║ Avg Response Time:  13 ms
║ Min Response Time:  5 ms
║ Max Response Time:  45 ms
╚════════════════════════════════════════════════════════╝
```

## Test Cases

The load tester includes 10 test cases with both incorrect and correct solutions:

| Question | Incorrect Solution | Correct Solution |
|----------|-------------------|------------------|
| Second Highest Salary | `SELECT MAX(salary) FROM Employee` | `SELECT MAX(salary) FROM Employee WHERE salary < (SELECT MAX(salary) FROM Employee)` |
| Duplicate Emails | `SELECT email FROM Person` | `SELECT email FROM Person GROUP BY email HAVING COUNT(*) > 1` |
| Department Highest Salary | `SELECT * FROM Department` | `SELECT d.Name AS Department, e.Name AS Employee, Salary FROM Employee e JOIN Department d ON e.DepartmentId = d.Id WHERE (e.DepartmentId, Salary) IN (SELECT DepartmentId, MAX(Salary) FROM Employee GROUP BY DepartmentId)` |
| ... | ... | ... |

## Running from Separate VM

To run the load test from a different VM:

1. **Build the load tester** on the test VM
2. **Ensure network connectivity** to the server VM
3. **Run with server IP**:

```bash
# On test VM
./load-tester http://192.168.1.100:8080 1000
```

## Performance Expectations

Based on the C++ implementation:

| Metric | Expected Value |
|--------|----------------|
| **Throughput** | 1000+ requests/second |
| **Avg Response Time** | 10-20ms |
| **Max Response Time** | < 100ms |
| **Success Rate** | 100% |
| **Memory** | ~1KB per session |

## Troubleshooting

### "Failed to create sessions"
- Check if server is running: `curl http://server:8080/health`
- Verify network connectivity: `ping server`
- Check firewall rules

### "Connection refused"
- Ensure server is running on target port
- Check server is binding to 0.0.0.0 (not localhost)

### High failure rate
- Check server logs for errors
- Verify DuckDB is working correctly
- Check if session timeout is too aggressive
