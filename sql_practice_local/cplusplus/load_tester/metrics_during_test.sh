#!/bin/bash
# Run this during load test to capture metrics

echo "Time  | CPU%  | MEM(MB) | Conn | Load 1m | Load 5m | Load 15m"
echo "------|-------|---------|------|---------|---------|----------"

for i in {1..20}; do
    # Get CPU from ps
    CPU=$(ps aux | grep "[s]ql-practice-server" | awk '{sum+=$3} END {if(NR>0) print sum/NR; else print "0"}')

    # Get Memory in MB
    MEM_KB=$(ps aux | grep "[s]ql-practice-server" | awk '{print $6}' | head -1)
    if [ -n "$MEM_KB" ]; then
        MEM_MB=$((MEM_KB / 1024))
    else
        MEM_MB=0
    fi

    # Get connection count
    CONN=$(ss -ant 2>/dev/null | grep :9000 | grep ESTAB | wc -l)

    # Get load average
    LOAD=$(cat /proc/loadavg | awk '{print $1 " " $2 " " $3}')

    printf "[%2d]   | %5s | %7s | %4s | %s\n" "$i" "$CPU" "$MEM_MB" "$CONN" "$LOAD"

    sleep 1
done
