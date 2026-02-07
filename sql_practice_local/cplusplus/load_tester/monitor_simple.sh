#!/bin/bash
echo "Time  CPU%    MEM(MB)  Conn  Load1m"
while true; do
    CPU=$(ps aux | grep "[s]ql-practice-server" | awk '{print $3}' | head -1)
    if [ -z "$CPU" ]; then CPU="N/A"; fi
    MEM_KB=$(ps aux | grep "[s]ql-practice-server" | awk '{print $6}' | head -1)
    if [ -n "$MEM_KB" ]; then
        MEM_MB=$((MEM_KB / 1024))
    else
        MEM_MB=0
    fi
    CONN=$(ss -ant | grep :9000 | grep ESTAB | wc -l 2>/dev/null || echo "0")
    LOAD=$(cat /proc/loadavg | awk '{print $1}')
    echo "[$(date +%H:%M:%S)] $CPU $MEM_MB $CONN $LOAD"
    sleep 1
done
