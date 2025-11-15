#!/usr/bin/env bash
set -euo pipefail
RND="test-nonexistent-$(date +%s%N).example"
if dig @127.0.0.1 -p 8053 "$RND" +time=3 | grep 'status: NXDOMAIN' >/dev/null; then
echo "PASS"
exit 0
else
echo "FAIL: expected NXDOMAIN"
exit 2
fi