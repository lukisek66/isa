#!/usr/bin/env bash
set -euo pipefail
if ! ping6 -c1 -w1 ::1 >/dev/null 2>&1; then
  echo "SKIP: no IPv6"
  exit 0
fi
OUT=$(dig @::1 -p 8053 example.com +short || true)
if [[ "$OUT" == "192.168.0.1" ]]; then
  echo "PASS"
else
  echo "FAIL IPv6 forward"
  exit 2
fi
