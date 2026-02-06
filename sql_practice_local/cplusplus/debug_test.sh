#!/bin/bash
echo "Testing single user flow..."
echo ""

# Login
echo "1. Login:"
LOGIN_RESPONSE=$(curl -s -X POST "http://localhost:9000/api/login" \
  -H "Content-Type: application/json" \
  -d '{"user_id":"debug_user_1"}')
echo "Login response: $LOGIN_RESPONSE"

SESSION_TOKEN=$(echo "$LOGIN_RESPONSE" | grep -o '"session_token":"[^"]*"' | cut -d'"' -f4)
echo "Session token: $SESSION_TOKEN"
echo ""

# Get question
echo "2. Get question:"
QUESTION_RESPONSE=$(curl -s "http://localhost:9000/api/question?session_token=$SESSION_TOKEN")
echo "Question response: $QUESTION_RESPONSE" | head -c 500
echo ""
echo ""

QUESTION_ID=$(echo "$QUESTION_RESPONSE" | grep -o '"question_id":[0-9]*' | head -1 | grep -o '[0-9]*$')
echo "Question ID: $QUESTION_ID"
echo ""

# Submit correct answer
echo "3. Submit answer:"
CORRECT_ANSWER=$(echo "$QUESTION_RESPONSE" | grep -o '"expected_sql":"[^"]*"' | cut -d'"' -f4 | sed 's/\\//g')
echo "Correct answer: $CORRECT_ANSWER"
echo ""

SUBMIT_RESPONSE=$(curl -s -X POST "http://localhost:9000/api/submit" \
  -H "Content-Type: application/json" \
  -d "{\"session_token\":\"$SESSION_TOKEN\",\"question_id\":$QUESTION_ID,\"answer\":\"$CORRECT_ANSWER\"}")
echo "Submit response: $SUBMIT_RESPONSE"
echo ""
