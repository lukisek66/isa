#!/usr/bin/env bash
set -euo pipefail
ROOT=$(cd "$(dirname "$0")/.." && pwd)
"$ROOT/dns" -s 127.0.0.1 -p 8053 -f "$ROOT/blocked.txt" -v &>/tmp/dns_timeout2.log &
PID=$!
sleep 0.5
dig @127.0.0.1 -p 8053 nosuchdomain-timeout.example +time=2 +tries=1 || true
sleep 6
# now send normal query to mock resolver (ensure dns was started in run_all with mock)
OUT=$(dig @127.0.0.1 -p 8053 example.com +short || true)
if [[ "$OUT" == "192.168.0.1" ]]; then
  kill $PID; wait $PID 2>/dev/null || true
  echo "PASS"
else
  kill $PID; wait $PID 2>/dev/null || true
  echo "FAIL: dns did not recover forwarding"
  exit 2
fi
