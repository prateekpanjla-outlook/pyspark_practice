#!/bin/bash

# Find where 95th Percentile Response Time < 3 seconds
# Tests from 25 users upward in smaller increments

SERVER_URL="http://192.168.56.10:9000"
LOAD_TESTER="/home/vagrant/load_test/build/load-tester"

echo "╔════════════════════════════════════════════════════════╗"
echo "║     95th Percentile Response Time Analysis                    ║"
echo "║     Finding threshold: 95th percentile < 3 seconds         ║"
echo "╚════════════════════════════════════════════════════════╝"
echo ""
echo "Server: $SERVER_URL"
echo "Starting: $(date)"
echo ""

echo "Users | 95th %ile | Avg Time | Max Time | Min Time | Success"
echo "------|-----------|----------|----------|----------|---------"

# Test sizes to try
SIZES=(25 50 75 100 125 150 175 200 250)

FOUND=0
for SIZE in "${SIZES[@]}"; do
    OUTPUT=$($LOAD_TESTER "$SERVER_URL" "$SIZE" 2>&1)

    # Extract metrics
    P95=$(echo "$OUTPUT" | grep "95th Percentile:" | grep -o '[0-9]* ms' | grep -o '[0-9]*' || echo "0")
    AVG=$(echo "$OUTPUT" | grep "Avg Response Time:" | grep -o '[0-9]* ms' | grep -o '[0-9]*' || echo "0")
    MAX=$(echo "$OUTPUT" | grep "Max Response Time:" | grep -o '[0-9]* ms' | grep -o '[0-9]*' || echo "0")
    MIN=$(echo "$OUTPUT" | grep "Min Response Time:" | grep -o '[0-9]* ms' | grep -o '[0-9]*' || echo "0")
    SUCCESS=$(echo "$OUTPUT" | grep "Successful:" | grep -o '[0-9]*' | tail -1 || echo "0")

    P95_SEC=$(echo "scale=2; $P95 / 1000" | bc)
    AVG_SEC=$(echo "scale=2; $AVG / 1000" | bc)
    MAX_SEC=$(echo "scale=2; $MAX / 1000" | bc)

    # Highlight if 95th percentile >= 3 seconds
    if [ $P95 -ge 3000 ]; then
        echo "$SIZE   |  ${P95_SEC}s   |  ${AVG_SEC}s   |  ${MAX_SEC}s   |  ${MIN}ms  |  $SUCCESS  *** THRESHOLD EXCEEDED ***"
        FOUND=1
        break
    else
        echo "$SIZE   |  ${P95_SEC}s   |  ${AVG_SEC}s   |  ${MAX_SEC}s   |  ${MIN}ms  |  $SUCCESS"
    fi

    sleep 1
done

echo ""
if [ $FOUND -eq 0 ]; then
    echo "✅ All tested sizes have 95th percentile < 3 seconds!"
    echo "   Up to 250 concurrent users tested successfully."
else
    echo ""
    echo "╔════════════════════════════════════════════════════════╗"
    echo "║               THRESHOLD EXCEEDED                         ║"
    echo "║   95th Percentile Response Time >= 3 seconds             ║"
    echo "╚════════════════════════════════════════════════════════╝"
fi

echo ""
echo "End Time: $(date)"
