#!/usr/bin/env bash
set -euo pipefail
OUT=$(dig @127.0.0.1 -p 8053 +time=3 www.google.com +short || true)
if echo "$OUT" | grep -E '^[0-9]+\.' >/dev/null; then
echo "PASS"
exit 0
else
echo "FAIL: no A record"
echo "$OUT"
exit 1
fi