#!/usr/bin/env bash
set -euo pipefail


DIG="dig @127.0.0.1 -p 8053 +time=2 +tries=1 www.google.com +short"
OUT=$(eval $DIG || true)
if [[ -z "$OUT" ]]; then
echo "FAIL: smoke test: no response"
exit 2
fi
echo "PASS: smoke (got: ${OUT%%$'\n'*})"