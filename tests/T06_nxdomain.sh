#!/usr/bin/env bash
set -euo pipefail
RND="nxdomain-test-$(date +%s%N).example"
if dig @127.0.0.1 -p 8053 "nxdomain.${RND}" +time=2 | grep 'status: NXDOMAIN' >/dev/null; then
  echo "PASS"
else
  echo "FAIL NXDOMAIN"
  exit 2
fi
