#!/bin/bash

# Dual Server Load Test - 5000 Total Concurrent Users
# Server 1: Port 9000 - 2500 users
# Server 2: Port 9001 - 2500 users

echo "╔════════════════════════════════════════════════════════╗"
echo "║     Dual Server Load Test - 5000 Concurrent Users     ║"
echo "╚════════════════════════════════════════════════════════╝"
echo ""
echo "Configuration:"
echo "  Server 1: http://192.168.56.10:9000 (2500 users)"
echo "  Server 2: http://192.168.56.10:9001 (2500 users)"
echo "  Total: 5000 concurrent users"
echo ""
echo "Starting both load tests simultaneously..."
echo ""
echo "Timestamp: $(date)"
echo ""

# Run both tests in parallel
bash /home/vagrant/load_test/run_test_server1.sh > /tmp/test_server1.log 2>&1 &
PID1=$!

bash /home/vagrant/load_test/run_test_server2.sh > /tmp/test_server2.log 2>&1 &
PID2=$!

echo "Load test processes started:"
echo "  Server 1 test: PID $PID1 (log: /tmp/test_server1.log)"
echo "  Server 2 test: PID $PID2 (log: /tmp/test_server2.log)"
echo ""
echo "Waiting for tests to complete..."
echo ""

# Wait for both to complete
wait $PID1
EXIT1=$?
wait $PID2
EXIT2=$?

echo ""
echo "╔════════════════════════════════════════════════════════╗"
echo "║              ALL TESTS COMPLETED                       ║"
echo "╚════════════════════════════════════════════════════════╝"
echo ""
echo "Server 1 Results:"
cat /tmp/test_server1.log
echo ""
echo "Server 2 Results:"
cat /tmp/test_server2.log
echo ""
echo "End Time: $(date)"
