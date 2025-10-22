#!/usr/bin/env bash
set -euo pipefail
OUT=$(dig @127.0.0.1 -p 8053 not-example.com +short || true)
if [[ "$OUT" == "1.2.3.4" ]]; then
  echo "PASS"
else
  echo "FAIL: expected 1.2.3.4 got '$OUT'"
  exit 2
fi
