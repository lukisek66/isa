#!/usr/bin/env python3
# Pošle dotaz s QNAME, kde druhá část používá pointer (komprese)
import socket


query = bytes([
0x12,0x34, 0x01,0x00, 0x00,0x01, 0x00,0x00, 0x00,0x00, 0x00,0x00,
# example.com at offset 12
0x07, ord('e'),ord('x'),ord('a'),ord('m'),ord('p'),ord('l'),ord('e'),
0x03, ord('c'),ord('o'),ord('m'), 0x00,
# question: www.C0 0C
0x03, ord('w'),ord('w'),ord('w'), 0xC0, 0x0C,
0x00,0x01, 0x00,0x01
])


s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.sendto(query, ('127.0.0.1', 8053))
s.close()
print('sent')
# Server's verbose log should show www.example.com