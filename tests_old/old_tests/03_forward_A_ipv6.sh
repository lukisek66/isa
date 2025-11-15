#!/usr/bin/env bash
set -euo pipefail
# Skip if no IPv6 loopback
if ! ping6 -c1 -w1 ::1 >/dev/null 2>&1; then
echo "SKIP: no IPv6"
exit 0
fi
OUT=$(dig @::1 -p 8053 +time=3 www.google.com +short || true)
if echo "$OUT" | grep -E '^[0-9a-fA-F:]+' >/dev/null; then
echo "PASS"
exit 0
else
echo "FAIL: IPv6 forward"
echo "$OUT"
exit 2
fi