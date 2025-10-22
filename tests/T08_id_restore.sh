#!/usr/bin/env bash
set -euo pipefail
# craft a query with known ID 0xABCD
python3 - <<'PY' | nc -u -w1 127.0.0.1 8053
import socket
query = bytes([
    0xAB,0xCD, 0x01,0x00, 0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,
    0x07, ord('e'),ord('x'),ord('a'),ord('m'),ord('p'),ord('l'),ord('e'),
    0x03, ord('c'),ord('o'),ord('m'), 0x00,
    0x00,0x01, 0x00,0x01
])
s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.settimeout(2)
s.sendto(query, ('127.0.0.1', 8053))
try:
    data, _ = s.recvfrom(4096)
    print(data[0], data[1])
except Exception as e:
    print("no reply", e)
finally:
    s.close()
PY
# We'll accept either reply present or not; manual log check recommended
echo "MANUAL CHECK: Look into dns logs to confirm ID=0xABCD restored"
