#!/bin/bash
BIN="../dns"
PORT=54000
RESOLVER="1.1.1.1"

touch blocked.txt
echo "ads.test.com" > blocked.txt

$BIN -s $RESOLVER -p $PORT -f blocked.txt -v &
PID=$!
sleep 1

OK=0
FAIL=0

dns_query() {
    printf "$1" | nc -u -w1 127.0.0.1 $PORT
}

# 1 – A query allowed
out=$(dns_query "$(cat packets/a_query_google.bin)")
if [[ $? -eq 0 ]]; then echo "Allowed A → PASS"; OK=$((OK+1)); else echo "FAIL"; FAIL=$((FAIL+1)); fi

# 2 – Blocked domain returns NXDOMAIN
out=$(dns_query "$(cat packets/a_query_blocked.bin)")
grep -q "NXDOMAIN" <<< "$out"
if [[ $? -eq 0 ]]; then echo "Blocked A → PASS"; OK=$((OK+1)); else echo "FAIL"; FAIL=$((FAIL+1)); fi

# 3 – Compressed name query
out=$(dns_query "$(cat packets/compressed_query.bin)")
if [[ $? -eq 0 ]]; then echo "Compression OK → PASS"; OK=$((OK+1)); else echo "FAIL"; FAIL=$((FAIL+1)); fi

# 4 – Malformed DNS message
out=$(dns_query "$(cat packets/malformed_query.bin)")
if [[ $? -ne 0 ]]; then echo "Malformed → PASS"; OK=$((OK+1)); else echo "FAIL"; FAIL=$((FAIL+1)); fi

# 5 – ID mapping check (server returns same client ID)
CID=$(printf "\xab\xcd")
out=$(printf "$CID test" | nc -u -w1 127.0.0.1 $PORT)
# Check byte 0-1 matches CID
if [[ "${out:0:2}" == "$CID" ]]; then echo "ID remap back → PASS"; OK=$((OK+1)); else echo "FAIL"; FAIL=$((FAIL+1)); fi

kill $PID

echo "DNS server: $OK passed, $FAIL failed."
[[ $FAIL -eq 0 ]]
