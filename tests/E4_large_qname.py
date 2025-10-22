#!/usr/bin/env python3
import socket
labels = []
remaining = 260
while remaining > 0:
    l = min(10, remaining-1)
    labels.append('a'*l)
    remaining -= (l+1)
qname_bytes = b''.join([bytes([len(l)])+l.encode() for l in labels])+b'\x00'
hdr = b'\x12\x36\x01\x00\x00\x01\x00\x00\x00\x00\x00\x00'
q = hdr + qname_bytes + b'\x00\x01\x00\x01'
s=socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
s.sendto(q,('127.0.0.1',8053))
s.close()
print("sent large qname")
