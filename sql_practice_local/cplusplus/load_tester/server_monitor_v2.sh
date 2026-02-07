#!/bin/bash
echo 'Time  | CPU%  | MEM(MB) | Threads | Conn | Load 1m | Load 5m | Load 15m'
echo '------|-------|---------|--------|------|---------|---------|----------'
count=0
while true; do
    count=$((count + 1))
    CPU=$(ps aux | grep '[s]ql-practice-server' | awk '{print $3}' | head -1)
    if [ -z "$CPU" ]; then CPU="N/A"; fi
    MEM_KB=$(ps aux | grep '[s]ql-practice-server' | awk '{print $6}' | head -1)
    if [ -n "$MEM_KB" ]; then
        MEM_MB=$((MEM_KB / 1024))
    else
        MEM_MB="N/A"
    fi
    PID=$(pgrep -f sql-practice-server | head -1)
    if [ -n "$PID" ]; then
        THREADS=$(ps -L -p $PID 2>/dev/null | wc -l)
    else
        THREADS="N/A"
    fi
    CONN=$(ss -ant | grep :9000 | grep ESTAB | wc -l)
    LOAD=$(cat /proc/loadavg | awk '{print $1 " " $2 " " $3}')
    printf "[%2d]   | %5s | %7s | %7s | %4s | %s\n" "$count" "$CPU" "$MEM_MB" "$THREADS" "$CONN" "$LOAD"
    sleep 1
done
