#!/bin/bash

echo "Starting 10K connection test..."
echo ""

# Start wrk in background
wrk -c 10000 -t 4 -d 60s --latency http://192.168.56.10:9000/health &
WRK_PID=$!

echo "WRK PID: $WRK_PID"
echo "Waiting for connections to establish..."
sleep 5

echo ""
echo "=== Connections on load tester VM ==="
ss -ant | grep ':9000' | grep ESTAB | wc -l

echo ""
echo "=== Connections on server VM ==="
ssh vagrant@192.168.56.10 "ss -ant | grep ':9000' | grep ESTAB | wc -l" 2>/dev/null || echo "Could not check server"

echo ""
echo "Waiting for test to complete..."
wait $WRK_PID

echo ""
echo "=== Test Complete ==="
