#!/bin/bash

# System Resource Monitoring During Load Test
# Identifies whether bottleneck is CPU, Memory, or Network

SERVER_URL="http://192.168.56.10:9000"
LOAD_TESTER="/home/vagrant/load_test/build/load-tester"
TEST_USERS=100  # Use size that shows clear behavior
TEST_DURATION=30  # Monitor for 30 seconds

echo "╔════════════════════════════════════════════════════════╗"
echo "║     Bottleneck Analysis - CPU vs Memory vs Network        ║"
echo "╚════════════════════════════════════════════════════════╝"
echo ""
echo "Configuration:"
echo "  Server: $SERVER_URL"
echo "  Test: $TEST_USERS concurrent users"
echo "  Monitor duration: ${TEST_DURATION}s"
echo ""
echo "Starting: $(date)"
echo ""

# Start monitoring in background
(
    for i in $(seq 1 $TEST_DURATION); do
        # Get top CPU consuming process
        CPU_INFO=$(ssh vagrant@192.168.56.10 "top -b -n 1 | grep sql-practice-server | head -1" 2>/dev/null || echo "0.0")

        # Get memory usage
        MEM_INFO=$(ssh vagrant@192.168.56.10 "ps aux | grep sql-practice-server | awk '{print \$6}'" 2>/dev/null)

        # Get server connections
        CONN_COUNT=$(ssh vagrant@192.168.56.10 "ss -ant | grep :9000 | grep ESTAB | wc -l" 2>/dev/null || echo "0")

        # Get load average
        LOAD_AVG=$(ssh vagrant@192.168.56.10 "cat /proc/loadavg | awk '{print \$1 \" \" \$2 \" \" \$3}'" 2>/dev/null || echo "N/A")

        # Get network queue
        NET_QUEUE=$(ssh vagrant@192.168.56.10 "cat /proc/net/netfilter/nf_conntrack | grep -E '^entries=[0-9]+' | cut -d= -f2" 2>/dev/null || echo "N/A")

        echo "[$i/$TEST_DURATION] CPU: ${CPU_INFO} | MEM: ${MEM_INFO} KB | Conn: $CONN_COUNT | Load: $LOAD_AVG"
        sleep 1
    done
) &
MONITOR_PID=$!

# Give monitor time to start
sleep 2

# Run load test in background
echo "Starting load test..."
echo ""
$LOAD_TESTER "$SERVER_URL" "$TEST_USERS" > /tmp/load_test_output.txt 2>&1 &
LOAD_PID=$!

# Wait for both to complete
wait $LOAD_PID 2>/dev/null

# Let monitor capture final state
sleep 2
kill $MONITOR_PID 2>/dev/null
wait $MONITOR_PID 2>/dev/null

echo ""
echo "Load test completed. Final resource state:"
echo ""

# Get detailed resource usage
echo "=== Server Process Details ==="
ssh vagrant@192.168.56.10 "echo '=== Process Info ===' && ps aux | grep sql-practice-server" 2>/dev/null
echo ""

echo "=== Memory Usage ==="
ssh vagrant@192.168.56.10 "free -h" 2>/dev/null
echo ""

echo "=== CPU Info ==="
ssh vagrant@192.168.56.10 "echo '=== CPU Count ===' && nproc && echo '=== Load Average ===' && cat /proc/loadavg" 2>/dev/null
echo ""

echo "=== Network Stats ==="
ssh vagrant@192.168.56.10 "echo '=== TCP Stats ===' && cat /proc/net/snmp | grep Ip: | grep -E 'Ext|Pass|Active'" 2>/dev/null
echo ""

echo "=== Disk I/O (if any) ==="
ssh vagrant@192.168.56.10 "iostat -x 1 1 2>/dev/null | grep -E 'Device|sql-practice|avg-cpu' || echo 'iostat not available'" 2>&1 | head -10
echo ""

echo "=== Load Test Results ==="
cat /tmp/load_test_output.txt | grep -E "Total Attempts|Successful|95th Percentile|Avg Response Time|Max Response Time"
