#!/usr/bin/env bash
set -euo pipefail
if dig @127.0.0.1 -p 8053 example.com +time=2 +tries=1 | grep 'status: REFUSED' >/dev/null; then
  # note: mock resolver returns A, but dns should REFUSE because example.com in blocked.txt
  echo "PASS"
else
  echo "FAIL: expected REFUSED"
  exit 2
fi
