#!/usr/bin/env bash
set -euo pipefail
# expect mock resolver to return 192.168.0.1 for example.com
OUT=$(dig @127.0.0.1 -p 8053 example.com +time=2 +tries=1 +short || true)
if [[ "$OUT" == "192.168.0.1" ]]; then
  echo "PASS"
  exit 0
else
  echo "FAIL forward A ipv4: got '$OUT'"
  exit 2
fi
