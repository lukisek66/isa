#!/usr/bin/env python3
# poslat kratky paket
import socket
s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.sendto(b'ABCD', ('127.0.0.1', 8053))
s.close()
print('sent short')