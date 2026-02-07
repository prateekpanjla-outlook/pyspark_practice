#!/bin/bash

TOKEN=$(curl -s -X POST "http://localhost:9000/api/login" -H "Content-Type: application/json" -d '{"user_id":"test"}' | grep -o '"session_token":"[^"]*"' | cut -d'"' -f4)
echo "Token: $TOKEN"
echo ""

echo "=== Check table contents ==="
curl -s -X POST "http://localhost:9000/api/execute" -H "Content-Type: application/json" -d "{\"session_token\":\"$TOKEN\",\"question_id\":\"q2\",\"user_sql\":\"SELECT * FROM Person;\"}" | head -c 800
echo ""
echo ""

echo "=== Run duplicate email query ==="
RESPONSE=$(curl -s -X POST "http://localhost:9000/api/execute" -H "Content-Type: application/json" -d "{\"session_token\":\"$TOKEN\",\"question_id\":\"q2\",\"user_sql\":\"SELECT email FROM Person GROUP BY email HAVING COUNT(*) > 1;\"}")
echo "$RESPONSE" | head -c 800
echo ""
echo ""

echo "=== Is correct? ==="
echo "$RESPONSE" | grep -o '"is_correct":[^,]*'
