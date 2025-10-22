#!/usr/bin/env python3
# vytvoří paket, kde pointer ukazuje na sebe -> test parsingu
import socket


# Header + qname: label_len=0xC0 pointer to offset 12 -> but offset 12 points to pointer -> loop
query = bytearray([
0x12,0x35, 0x01,0x00, 0x00,0x01, 0x00,0x00, 0x00,0x00, 0x00,0x00,
0xC0, 0x0C, # pointer to offset 12
0x00,0x01, 0x00,0x01
])


s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.sendto(query, ('127.0.0.1', 8053))
s.close()
print('sent pointer loop')