#!/usr/bin/env python3
# label length that overflows message
import socket
buf = bytearray([
0x12,0x36, 0x01,0x00, 0x00,0x01, 0x00,0x00, 0x00,0x00, 0x00,0x00,
0xFF # label length 255 but no data
])


s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.sendto(buf, ('127.0.0.1', 8053))
s.close()
print('sent invalid label')