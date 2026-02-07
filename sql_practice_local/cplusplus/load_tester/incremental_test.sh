#!/bin/bash

# Incremental Load Test - Find threshold where Max Response Time >= 3 seconds
# Tests from 100 users upward

SERVER_URL="http://192.168.56.10:9000"
LOAD_TESTER="/home/vagrant/load_test/build/load-tester"
LOG_FILE="/tmp/incremental_test_results.txt"

echo "╔════════════════════════════════════════════════════════╗"
echo "║     Incremental Load Test - Max Response Time Analysis ║"
echo "╚════════════════════════════════════════════════════════╝"
echo ""
echo "Server: $SERVER_URL"
echo "Criterion: Max Response Time >= 3 seconds"
echo "Starting: $(date)"
echo ""

# Test sizes to try (100, 200, 300, 400, 500, 600, 700, 800, 900, 1000)
SIZES=(100 200 300 400 500 600 700 800 900 1000)

FOUND=0
for SIZE in "${SIZES[@]}"; do
    echo "Testing with $SIZE concurrent users..."

    # Run the test and capture output
    OUTPUT=$($LOAD_TESTER "$SERVER_URL" "$SIZE" 2>&1)

    # Extract max response time
    MAX_TIME=$(echo "$OUTPUT" | grep "Max Response Time:" | grep -o '[0-9]* ms' | grep -o '[0-9]*')

    if [ -n "$MAX_TIME" ]; then
        MAX_TIME_SEC=$(echo "scale=2; $MAX_TIME / 1000" | bc)
        echo "  Max Response Time: ${MAX_TIME_SEC} seconds"

        # Check if >= 3 seconds
        if (( $(echo "$MAX_TIME_SEC >= 3.0" | bc -l) )); then
            echo ""
            echo "╔════════════════════════════════════════════════════════╗"
            echo "║               THRESHOLD FOUND!                         ║"
            echo "╠════════════════════════════════════════════════════════╣"
            echo "║ Concurrent Users:                                    $SIZE ║"
            echo "║ Max Response Time:                              ${MAX_TIME_SEC} sec ║"
            echo "╚════════════════════════════════════════════════════════╝"
            FOUND=1

            # Save full results
            echo "" >> "$LOG_FILE"
            echo "=== Test with $SIZE users ===" >> "$LOG_FILE"
            echo "$OUTPUT" >> "$LOG_FILE"
            echo "" >> "$LOG_FILE"
            break
        fi
    else
        echo "  Failed to get response time"
    fi

    echo ""
    sleep 2
done

if [ $FOUND -eq 0 ]; then
    echo "No threshold found up to 1000 users. Max response time stayed below 3 seconds."
fi

echo ""
echo "End Time: $(date)"
echo "Full results saved to: $LOG_FILE"
