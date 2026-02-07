#!/bin/bash
echo "Testing query manually..."
echo ""

TOKEN=$(curl -s -X POST "http://localhost:9000/api/login" -H "Content-Type: application/json" -d '{"user_id":"manual_test"}' | grep -o '"session_token":"[^"]*"' | cut -d'"' -f4)

echo "Token: $TOKEN"
echo ""

echo "=== Question ==="
curl -s "http://localhost:9000/api/questions/1" | head -c 800
echo ""
echo ""

echo "=== Submitting correct answer ==="
RESPONSE=$(curl -s -X POST "http://localhost:9000/api/execute" -H "Content-Type: application/json" -d "{\"session_token\":\"$TOKEN\",\"question_id\":\"1\",\"user_sql\":\"SELECT Email FROM Person GROUP BY Email HAVING COUNT(*) > 1;\"}")
echo "$RESPONSE" | head -c 800
echo ""
