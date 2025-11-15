#!/usr/bin/env python3
import socket,time
s=socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
for i in range(2000):
    id_hi=(i>>8)&0xFF; id_lo=i&0xFF
    q = bytes([id_hi,id_lo,0x01,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00]) + \
        b'\x07example\x03com\x00\x00\x01\x00\x01'
    s.sendto(q,('127.0.0.1',8053))
    if i%200==0: time.sleep(0.1)
print("flood sent")
