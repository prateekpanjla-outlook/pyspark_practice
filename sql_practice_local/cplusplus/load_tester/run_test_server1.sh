#!/bin/bash

# Load Test Script for Server 1 (Port 9000)
# 2500 concurrent users

SERVER_URL="http://192.168.56.10:9000"
NUM_USERS=2500

echo "========================================================="
echo "     Load Test - Server 1 (Port 9000)                  "
echo "========================================================="
echo "Server URL: $SERVER_URL"
echo "Concurrent Users: $NUM_USERS"
echo "Total Requests: $((NUM_USERS * 2))"
echo ""

/home/vagrant/load_test/build/load-tester "$SERVER_URL" "$NUM_USERS"

echo ""
echo "========================================================="
echo "     Test Completed - Server 1                         "
echo "========================================================="
