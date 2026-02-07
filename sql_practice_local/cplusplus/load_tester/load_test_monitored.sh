#!/bin/bash

# SQL Practice Server - 1000 Concurrent Connection Load Test with Monitoring
# Tests from external load tester VM

SERVER_URL="${1:-http://192.168.56.10:9000}"
NUM_USERS="${2:-1000}"
MAX_CONCURRENT=1000

echo "========================================================="
echo "     SQL Practice Server - Load Testing (1000 Concurrent) "
echo "========================================================="
echo "Configuration:"
echo "  Server URL: $SERVER_URL"
echo "  Total Users: $NUM_USERS"
echo "  Max Concurrent: $MAX_CONCURRENT"
echo ""

# Results file
RESULTS_FILE=$(mktemp)
export SERVER_URL RESULTS_FILE

# Counter variables
TOTAL_ATTEMPTS=0
SUCCESSFUL=0
FAILED=0
CORRECT=0
WRONG=0
TOTAL_RESPONSE_TIME=0
MIN_RESPONSE_TIME=999999
MAX_RESPONSE_TIME=0

# Function to test a single user
test_user() {
    local i=$1
    local USER_ID="external_user_$i"

    # Login
    local START=$(date +%s%3N)
    local LOGIN_RESPONSE=$(curl -s -X POST "$SERVER_URL/api/login" \
        -H "Content-Type: application/json" \
        -d "{\"user_id\":\"$USER_ID\"}" \
        --max-time 30 2>/dev/null)
    local LOGIN_TIME=$(($(date +%s%3N) - START))

    # Check if login succeeded
    local SESSION_TOKEN=$(echo "$LOGIN_RESPONSE" | grep -o '"session_token":"[^"]*"' | cut -d'"' -f4)

    if [ -z "$SESSION_TOKEN" ]; then
        echo "$i|FAIL|0|login_failed|0" >> "$RESULTS_FILE"
        return
    fi

    # Test Question ID q2: Duplicate Emails
    # Wrong answer first
    curl -s -X POST "$SERVER_URL/api/execute" \
        -H "Content-Type: application/json" \
        -d "{\"session_token\":\"$SESSION_TOKEN\",\"question_id\":\"q2\",\"user_sql\":\"SELECT * FROM table;\"}" \
        --max-time 30 > /dev/null 2>&1

    # Correct answer
    local START2=$(date +%s%3N)
    local SUBMIT_RESPONSE=$(curl -s -X POST "$SERVER_URL/api/execute" \
        -H "Content-Type: application/json" \
        -d "{\"session_token\":\"$SESSION_TOKEN\",\"question_id\":\"q2\",\"user_sql\":\"SELECT email FROM Person GROUP BY email HAVING COUNT(*) > 1;\"}" \
        --max-time 30 2>/dev/null)
    local SUBMIT_TIME=$(($(date +%s%3N) - START2))

    local TOTAL_TIME=$((LOGIN_TIME + SUBMIT_TIME))
    local IS_CORRECT=$(echo "$SUBMIT_RESPONSE" | grep -o '"is_correct":true')

    if [ -n "$IS_CORRECT" ]; then
        echo "$i|SUCCESS|$TOTAL_TIME|1" >> "$RESULTS_FILE"
    else
        echo "$i|SUCCESS|$TOTAL_TIME|0" >> "$RESULTS_FILE"
    fi
}

export -f test_user

# Start the load test in background
echo "Starting load test with $NUM_USERS concurrent users..."
echo "Monitoring connections in parallel..."
echo ""

# Start xargs in background
seq 1 $NUM_USERS | xargs -P $MAX_CONCURRENT -I {} bash -c 'test_user "$@"' _ {} &
XARGS_PID=$!

# Monitoring loop
MAX_CONNECTIONS=0
sleep 2  # Let connections build up

for i in {1..60}; do
    sleep 1

    # Count outgoing connections to server (local monitoring)
    CONN_COUNT=$(ss -ant | grep ':9000' | grep ESTAB | wc -l)

    if [ $CONN_COUNT -gt $MAX_CONNECTIONS ]; then
        MAX_CONNECTIONS=$CONN_COUNT
    fi

    # Show progress
    RUNNING=$(ps aux | grep xargs | grep -v grep | grep -v "xargs -P" | wc -l)
    echo "[$i/20] Active connections: $CONN_COUNT | Max observed: $MAX_CONNECTIONS | xargs processes: $RUNNING"

    # Check if xargs is still running
    if ! ps -p $XARGS_PID > /dev/null 2>&1; then
        echo ""
        echo "Load test completed!"
        break
    fi
done

# Wait for xargs to finish
wait $XARGS_PID 2>/dev/null

# Give time for final writes
sleep 3
echo ""
echo "All requests completed. Processing results..."
echo ""

# Process results
while IFS='|' read -r USER_ID STATUS TIME IS_CORRECT; do
    TOTAL_ATTEMPTS=$((TOTAL_ATTEMPTS + 1))

    if [ "$STATUS" = "SUCCESS" ]; then
        SUCCESSFUL=$((SUCCESSFUL + 1))
        TOTAL_RESPONSE_TIME=$((TOTAL_RESPONSE_TIME + TIME))

        if [ $TIME -lt $MIN_RESPONSE_TIME ]; then
            MIN_RESPONSE_TIME=$TIME
        fi
        if [ $TIME -gt $MAX_RESPONSE_TIME ]; then
            MAX_RESPONSE_TIME=$TIME
        fi

        if [ "$IS_CORRECT" = "1" ]; then
            CORRECT=$((CORRECT + 1))
        else
            WRONG=$((WRONG + 1))
        fi
    else
        FAILED=$((FAILED + 1))
    fi
done < "$RESULTS_FILE"

# Calculate average
if [ $SUCCESSFUL -gt 0 ]; then
    AVG_RESPONSE_TIME=$((TOTAL_RESPONSE_TIME / SUCCESSFUL))
    ACCURACY=$(awk "BEGIN {printf \"%.2f\", $CORRECT * 100 / $SUCCESSFUL}")
else
    AVG_RESPONSE_TIME=0
    ACCURACY="0.00"
fi

echo "========================================================="
echo "           LOAD TEST RESULTS                              "
echo "--------------------------------------------------------"
printf "Total Attempts:                                         %d\n" $TOTAL_ATTEMPTS
printf "Successful:                                             %d\n" $SUCCESSFUL
printf "Failed:                                                  %d\n" $FAILED
printf "Correct Answers:                                         %d\n" $CORRECT
printf "Wrong Answers:                                          %d\n" $WRONG
echo "--------------------------------------------------------"
printf "Avg Response Time:                                    %d ms\n" $AVG_RESPONSE_TIME
printf "Min Response Time:                                    %d ms\n" $MIN_RESPONSE_TIME
printf "Max Response Time:                                    %d ms\n" $MAX_RESPONSE_TIME
printf "Accuracy:                                              %s%%\n" "$ACCURACY"
printf "Max Concurrent Connections Observed:                       %d\n" $MAX_CONNECTIONS
echo "========================================================="

# Cleanup
rm -f "$RESULTS_FILE"
