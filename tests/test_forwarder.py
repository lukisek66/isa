import subprocess
import struct
import time
import socket

# forwarder běží uvnitř binárky – voláme přes malý testovací program
import forwarder_test_lib as fwd

# 1 – ID generation increments
id1 = fwd.newid()
id2 = fwd.newid()
print("Increment:", "PASS" if id2 == id1+1 else "FAIL")

# 2 – ID resets at overflow
for _ in range(65535 - id2 + 2):
    fwd.newid()
id_over = fwd.newid()
print("Overflow reset:", "PASS" if id_over == 1 else "FAIL")

# 3 – Map client ID → server ID
client_id = 1234
server_id = fwd.map_id(client_id)
print("Map ID:", "PASS" if server_id != client_id else "FAIL")

# 4 – Reverse map
reverse = fwd.reverse_map(server_id)
print("Reverse map:", "PASS" if reverse == client_id else "FAIL")

# 5 – Timeout removal
id_timeout = fwd.map_id(5555)
time.sleep(3)   # forwarder timeout = 2s
removed = fwd.reverse_map(id_timeout)
print("Timeout removal:", "PASS" if removed is None else "FAIL")
