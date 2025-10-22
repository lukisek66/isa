#!/usr/bin/env python3
import socket
import sys

if len(sys.argv) < 2:
    print(f"Usage: {sys.argv[0]} PORT")
    sys.exit(1)

PORT = int(sys.argv[1])
HOST = "127.0.0.1"  # bind na IPv4 localhost

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((HOST, PORT))

print(f"Mock resolver běží na {HOST}:{PORT}")

try:
    while True:
        data, addr = sock.recvfrom(4096)
        print(f"Přijato {len(data)} bajtů od {addr}")
        # Odpověď: jednoduše vrátíme stejný paket zpět
        sock.sendto(data, addr)
except KeyboardInterrupt:
    print("\nKonec.")
finally:
    sock.close()
