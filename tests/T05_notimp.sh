#!/usr/bin/env bash
set -euo pipefail
if dig @127.0.0.1 -p 8053 example.com AAAA +time=2 | grep 'status: NOTIMP' >/dev/null; then
  echo "PASS"
else
  echo "FAIL: expected NOTIMP"
  exit 2
fi
