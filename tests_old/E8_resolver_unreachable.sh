#!/usr/bin/env bash
set -euo pipefail
# start dns pointing to 127.0.0.1:9999 (no resolver there)
ROOT=$(cd "$(dirname "$0")/.." && pwd)
"$ROOT/dns" -s 127.0.0.1 -p 8053 -f "$ROOT/blocked.txt" -v &>/tmp/dns_unreach.log &
PID=$!
sleep 0.5
# send one query (will time out)
dig @127.0.0.1 -p 8053 example.com +time=2 +tries=1 || true
sleep 0.5
if ps -p $PID >/dev/null; then
  kill $PID
  wait $PID 2>/dev/null || true
  echo "PASS"
else
  echo "FAIL: dns crashed"
  exit 2
fi
