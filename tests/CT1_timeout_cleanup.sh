#!/usr/bin/env bash
set -euo pipefail
ROOT=$(cd "$(dirname "$0")/.." && pwd)
"$ROOT/dns" -s 127.0.0.1 -p 8053 -f "$ROOT/blocked.txt" -v &>/tmp/dns_timeout.log &
PID=$!
sleep 0.5
# send a query; upstream (127.0.0.1:9999) is not responding
dig @127.0.0.1 -p 8053 nosuchdomain-timeout.example +time=2 +tries=1 || true
# wait some seconds for possible cleanup to run
sleep 6
if ps -p $PID >/dev/null; then
  kill $PID
  wait $PID 2>/dev/null || true
  echo "PASS"
else
  echo "FAIL: server crashed"
  exit 2
fi
