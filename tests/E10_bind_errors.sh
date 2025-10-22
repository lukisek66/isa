#!/usr/bin/env bash
set -euo pipefail
# try to bind privileged port without sudo should fail
if ./dns -s 127.0.0.1 -p 53 -f blocked.txt >/dev/null 2>&1; then
  echo "FAIL: should not bind to port 53 without privileges"
  exit 2
else
  echo "PASS"
fi
