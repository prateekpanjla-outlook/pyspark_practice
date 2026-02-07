#!/bin/bash

# Load Test Script for Server 2 (Port 9001)
# 2500 concurrent users

SERVER_URL="http://192.168.56.10:9001"
NUM_USERS=2500

echo "========================================================="
echo "     Load Test - Server 2 (Port 9001)                  "
echo "========================================================="
echo "Server URL: $SERVER_URL"
echo "Concurrent Users: $NUM_USERS"
echo "Total Requests: $((NUM_USERS * 2))"
echo ""

/home/vagrant/load_test/build/load-tester "$SERVER_URL" "$NUM_USERS"

echo ""
echo "========================================================="
echo "     Test Completed - Server 2                         "
echo "========================================================="
