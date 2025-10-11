import socket

# --- Binární DNS dotaz typu A pro example.com ---
# ID: 0x1234
# Flags: standard query
# QDCOUNT = 1, ANCOUNT=0, NSCOUNT=0, ARCOUNT=0
# QNAME = www.example.com (s pointerem pro kompresi)
# QTYPE = 1 (A), QCLASS = 1 (IN)

# Offsety: pointer 0xC0 0x0C ukazuje na začátek 'example.com'
query = bytes([
    0x12,0x34,       # ID
    0x01,0x00,       # flags: standard query
    0x00,0x01,       # QDCOUNT = 1
    0x00,0x00,       # ANCOUNT = 0
    0x00,0x00,       # NSCOUNT = 0
    0x00,0x00,       # ARCOUNT = 0
    # QNAME: example.com
    0x07, ord('e'),ord('x'),ord('a'),ord('m'),ord('p'),ord('l'),ord('e'),
    0x03, ord('c'),ord('o'),ord('m'),
    0x00,             # konec QNAME
    # QTYPE a QCLASS
    0x00,0x01,        # QTYPE=A
    0x00,0x01         # QCLASS=IN
])

# --- Konfigurace cíle ---
UDP_IP = "127.0.0.1"  # nebo '::1' pro IPv6
UDP_PORT = 8053       # port, na kterém tvůj server poslouchá

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.sendto(query, (UDP_IP, UDP_PORT))
sock.close()

print(f"Posláno {len(query)} bajtů na {UDP_IP}:{UDP_PORT}")
