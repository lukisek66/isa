import subprocess
import sys

def run(domain):
    p = subprocess.Popen(
        ["../dns_test_filter", domain],  # malý testovací bin který jen vypíše BLOCKED/OK
        stdout=subprocess.PIPE
    )
    return p.stdout.read().decode().strip()

def expect(desc, domain, expected):
    result = run(domain)
    ok = (result == expected)
    print(f"{desc}: {domain} -> {result} ... {'PASS' if ok else 'FAIL'}")
    if not ok:
        sys.exit(1)

# 1 – exact match
expect("Exact match", "ads.google.com", "BLOCKED")

# 2 – subdomain match
expect("Subdomain match", "track.ads.google.com", "BLOCKED")

# 3 – uppercase input
expect("Uppercase", "Ads.Google.com", "BLOCKED")

# 4 – trailing dot
expect("Trailing dot", "ads.google.com.", "BLOCKED")

# 5 – clean domain not blocked
expect("Allowed domain", "example.com", "OK")
