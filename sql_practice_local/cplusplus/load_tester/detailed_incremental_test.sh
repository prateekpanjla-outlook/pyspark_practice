#!/bin/bash

# Detailed Incremental Load Test - Find exact threshold
# Tests from 25 users upward in smaller increments

SERVER_URL="http://192.168.56.10:9000"
LOAD_TESTER="/home/vagrant/load_test/build/load-tester"

echo "╔════════════════════════════════════════════════════════╗"
echo "║     Detailed Incremental Load Test                      ║"
echo "║     Finding Max Response Time >= 3 seconds threshold    ║"
echo "╚════════════════════════════════════════════════════════╝"
echo ""
echo "Server: $SERVER_URL"
echo "Starting: $(date)"
echo ""

# Test sizes: 25, 50, 75, 100, 125, 150
SIZES=(25 50 75 100 125 150)

echo "Users  | Max Time | Avg Time | Min Time | Success | Throughput"
echo "-------|----------|----------|----------|---------|-----------"

for SIZE in "${SIZES[@]}"; do
    OUTPUT=$($LOAD_TESTER "$SERVER_URL" "$SIZE" 2>&1)

    # Extract metrics
    MAX_TIME=$(echo "$OUTPUT" | grep "Max Response Time:" | grep -o '[0-9]* ms' | grep -o '[0-9]*' || echo "0")
    AVG_TIME=$(echo "$OUTPUT" | grep "Avg Response Time:" | grep -o '[0-9]* ms' | grep -o '[0-9]*' || echo "0")
    MIN_TIME=$(echo "$OUTPUT" | grep "Min Response Time:" | grep -o '[0-9]* ms' | grep -o '[0-9]*' || echo "0")
    SUCCESSFUL=$(echo "$OUTPUT" | grep "Successful:" | grep -o '[0-9]*' | tail -1 || echo "0")
    THROUGHPUT=$(echo "$OUTPUT" | grep "Throughput:" | grep -o '[0-9.]*' | head -1 || echo "0")

    MAX_SEC=$(echo "scale=2; $MAX_TIME / 1000" | bc)
    AVG_SEC=$(echo "scale=2; $AVG_TIME / 1000" | bc)

    # Highlight row if max time >= 3 seconds
    if (( $(echo "$MAX_TIME >= 3000" | bc -l) )); then
        echo "$SIZE   |  ${MAX_SEC}s   |  ${AVG_SEC}s   |  ${MIN_TIME}ms  |  $SUCCESSFUL  |  ${THROUGHPUT}/s  *** THRESHOLD ***"
    else
        echo "$SIZE   |  ${MAX_SEC}s   |  ${AVG_SEC}s   |  ${MIN_TIME}ms  |  $SUCCESSFUL  |  ${THROUGHPUT}/s"
    fi

    # If we found threshold and max is >= 3 sec, we can stop
    if [ $MAX_TIME -ge 3000 ] && [ $SIZE -gt 50 ]; then
        break
    fi

    sleep 1
done

echo ""
echo "End Time: $(date)"
