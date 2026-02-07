#!/bin/bash

# Run this script ON THE SERVER VM (192.168.56.10)
# It monitors the sql-practice-server process resources

echo "╔════════════════════════════════════════════════════════╗"
echo "║     Resource Monitor - Run on Server VM                 ║"
echo "║     (ssh vagrant@192.168.56.10 then run this)          ║"
echo "╚════════════════════════════════════════════════════════╝"
echo ""
echo "Monitoring sql-practice-server process..."
echo "Press Ctrl+C to stop"
echo ""

echo "Time  | CPU%  | MEM(MB) | Threads | Conn | Load 1m | Load 5m | Load 15m"
echo "------|-------|---------|--------|------|---------|---------|----------"

count=0
while true; do
    count=$((count + 1))

    # Get CPU usage (from ps)
    CPU=$(ps aux | grep '[s]ql-practice-server' | awk '{print $3}')

    # Get memory (RSS in KB, convert to MB)
    MEM_KB=$(ps aux | grep '[s]ql-practice-server' | awk '{print $6}')
    if [ -n "$MEM_KB" ]; then
        MEM_MB=$((MEM_KB / 1024))
    else
        MEM_MB="N/A"
    fi

    # Get number of threads
    THREADS=$(ps -L -p $(pgrep -f sql-practice-server) | wc -l 2>/dev/null || echo "N/A")

    # Get connections
    CONN=$(ss -ant | grep :9000 | grep ESTAB | wc -l)

    # Get load average
    LOAD=$(cat /proc/loadavg | awk '{print $1 " " $2 " " $3}')

    printf "[%2d]   | %5s | %7s | %7s | %4s | %s\n" \
           "$count" "$CPU" "$MEM_MB" "$THREADS" "$CONN" "$LOAD"

    sleep 1
done
