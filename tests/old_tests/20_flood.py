#!/usr/bin/env python3
# rychlý flood test (nebezpečné, pouzivejte s rozmyslem)
import socket, time
s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
for i in range(2000):
id_hi = (i >> 8) & 0xFF
id_lo = i & 0xFF
q = bytes([
id_hi,id_lo, 0x01,0x00, 0x00,0x01, 0x00,0x00, 0x00,0x00, 0x00,0x00,
0x07, ord('e'),ord('x'),ord('a'),ord('m'),ord('p'),ord('l'),ord('e'),
0x03, ord('c'),ord('o'),ord('m'), 0x00, 0x00,0x01, 0x00,0x01
])
s.sendto(q, ('127.0.0.1', 8053))
if i % 200 == 0:
time.sleep(0.1)
print('flood sent')