#!/usr/bin/env python3
import socket, time
# Compose message: header + example.com + 0 + www + pointer to example (offset 12)
query = bytes([
    0x12,0x34, 0x01,0x00, 0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,
    0x07, ord('e'),ord('x'),ord('a'),ord('m'),ord('p'),ord('l'),ord('e'),
    0x03, ord('c'),ord('o'),ord('m'), 0x00,
    0x03, ord('w'),ord('w'),ord('w'), 0xC0,0x0C,
    0x00,0x01, 0x00,0x01
])
s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.settimeout(2)
s.sendto(query, ('127.0.0.1', 8053))
# optionally receive (if forwarder replies) - but we just ensure send
try:
    data, _ = s.recvfrom(4096)
    print("got reply len", len(data))
except socket.timeout:
    print("no reply (ok)")
s.close()
print("sent")
