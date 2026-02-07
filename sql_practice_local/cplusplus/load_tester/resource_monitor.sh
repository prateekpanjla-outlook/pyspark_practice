#!/bin/bash

# Quick Resource Monitor - Run during load test to identify bottleneck

echo "╔════════════════════════════════════════════════════════╗"
echo "║     Resource Monitor - CPU / Memory / Network               ║"
echo "╚════════════════════════════════════════════════════════╝"
echo ""
echo "Monitoring server: 192.168.56.10:9000"
echo "Press Ctrl+C to stop"
echo ""

echo "Time  | CPU%  | MEM(MB) | Threads | Conn | Load 1m | Net-Recv | Net-Send |  TCP-Q"
echo "------|-------|---------|--------|------|---------|---------|---------|--------"

count=0
while true; do
    count=$((count + 1))

    # Get CPU usage (from ps)
    CPU=$(ssh vagrant@192.168.56.10 "ps aux | grep '[s]ql-practice-server' | awk '{print \$3}'" 2>/dev/null)

    # Get memory (RSS in KB, convert to MB)
    MEM_KB=$(ssh vagrant@192.168.56.10 "ps aux | grep '[s]ql-practice-server' | awk '{print \$6}'" 2>/dev/null)
    if [ -n "$MEM_KB" ]; then
        MEM_MB=$((MEM_KB / 1024))
    else
        MEM_MB="N/A"
    fi

    # Get number of threads
    THREADS=$(ssh vagrant@192.168.56.10 "ps -L -p \$(pgrep -f sql-practice-server) | wc -l" 2>/dev/null || echo "N/A")

    # Get connections
    CONN=$(ssh vagrant@192.168.56.10 "ss -ant | grep :9000 | grep ESTAB | wc -l" 2>/dev/null || echo "0")

    # Get load average (1 min)
    LOAD=$(ssh vagrant@192.168.56.10 "cut -d' ' -f1 /proc/loadavg" 2>/dev/null || echo "N/A")

    # Get network stats (bytes received)
    NET_RECV=$(ssh vagrant@192.168.56.10 "cat /proc/net/dev | grep enp0s8 | awk '{print \$2}'" 2>/dev/null | awk '{print int(\$1/1024)}' || echo "0")
    NET_SEND=$(ssh vagrant@192.168.56.10 "cat /proc/net/dev | grep enp0s8 | awk '{print \$10}'" 2>/dev/null | awk '{print int(\$1/1024)}' || echo "0")

    # Get TCP queue depth (SYN_RECV)
    TCP_Q=$(ssh vagrant@192.168.56.10 "netstat -s | grep 'queue length' | awk '{print \$4}' 2>/dev/null || echo "N/A")

    printf "[%2d]   | %5s | %7s | %7s | %4s | %7s | %8sKB | %8sKB | %s\n" \
           "$count" "$CPU" "$MEM_MB" "$THREADS" "$CONN" "$LOAD" "$NET_RECV" "$NET_SEND" "$TCP_Q"

    sleep 1
done
