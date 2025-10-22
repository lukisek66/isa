#!/usr/bin/env bash
set -euo pipefail
OUT=$(dig @127.0.0.1 -p 8053 example.com +time=3 +noall +answer +stats || true)
# Look for REFUSED in header stats
if dig @127.0.0.1 -p 8053 example.com +time=3 +tries=1 | grep 'status: REFUSED' >/dev/null; then
echo "PASS"
exit 0
else
echo "FAIL: expected REFUSED"
exit 2
fi