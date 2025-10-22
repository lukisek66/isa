#!/usr/bin/env bash
set -euo pipefail
# Tento test nepředpokládá, že implementuješ cleanup. Test ověří, že server nespadne
# pokud upstream neodpovídá.


# Spusť server s upstreamem na adresu, kde neni DNS
ROOT=$(cd "$(dirname "$0")/.." && pwd)
DNS_BIN="$ROOT/dns"
BLOCKED="$ROOT/blocked.txt"


"$DNS_BIN" -s 127.0.0.1 -p 8053 -f "$BLOCKED" -v &
PID=$!
sleep 1
# pošli dotaz na server, upstream (127.0.0.1) neposkytne odpověď
dig @127.0.0.1 -p 8053 nosuchdomain-for-timeout-test.example +time=2 +tries=1 >/dev/null || true
# počkej chvíli, zkontroluj, že proces ještě běží
if ps -p $PID > /dev/null; then
echo "PASS: server stále běží (cleanup not implemented or ok)"
kill $PID
wait $PID 2>/dev/null || true
exit 0
else
echo "FAIL: server crashed"
exit 2
fi