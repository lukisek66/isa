#!/usr/bin/env bash
set -euo pipefail
ROOT=$(cd "$(dirname "$0")/.." && pwd)
"$ROOT/dns" -s 127.0.0.1 -p 8053 -f "$ROOT/blocked.txt" -v &>/tmp/dns_sig.log &
PID=$!
sleep 0.5
kill -INT $PID
sleep 0.2
if ps -p $PID >/dev/null; then
  echo "FAIL: did not shutdown"
  kill $PID
  exit 2
else
  echo "PASS"
fi
