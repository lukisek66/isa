#!/usr/bin/env bash
set -euo pipefail
ROOT=$(cd "$(dirname "$0")/.." && pwd)
DNS_BIN="$ROOT/dns"
BLOCKED="$ROOT/blocked.txt"

# Missing required args
if "$DNS_BIN" >/dev/null 2>&1; then
  echo "FAIL: expected failure when missing args"
  exit 2
fi

# Valid combos
"$DNS_BIN" -s 127.0.0.1 -p 8054 -f "$BLOCKED" -v &>/dev/null &
PID=$!
sleep 0.2
if ps -p $PID >/dev/null; then
  kill $PID
  wait $PID 2>/dev/null || true
else
  echo "FAIL: server didn't start with valid args"
  exit 2
fi
echo "PASS"
