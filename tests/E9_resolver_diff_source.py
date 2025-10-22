#!/usr/bin/env python3
import socket, time
# send a query to dns server then send back a response from different port to emulate weird upstream
# This test requires the dns server to have forwarded something; here we directly send a response to dns
# For simplicity: we craft a response and send from random port to localhost: (dns should accept by ID)
resp = bytes([0x00,0x01,0x81,0x80,0x00,0x01,0x00,0x01,0x00,0x00,0x00,0x00,
              0x07])+b'example'+b'\x03com\x00\x00\x01\x00\x01' + b'\xC0\x0C\x00\x01\x00\x01\x00\x00\x01\x2C\x00\x04\x7f\x00\x00\x01'
s=socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
s.bind(('127.0.0.1',0))
s.sendto(resp, ('127.0.0.1', 8053))
print("sent response from different source port")
