#!/usr/bin/env bash
set -euo pipefail
if dig @127.0.0.1 -p 8053 example.com AAAA +time=3 | grep 'status: NOTIMP' >/dev/null; then
echo "PASS"
exit 0
else
echo "FAIL: expected NOTIMP"
exit 2
fi