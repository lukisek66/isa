#!/bin/bash
BIN="../dns"
OK=0
FAIL=0

run_test() {
    DESC=$1
    CMD=$2
    EXPECT=$3

    OUT=$($CMD 2>&1)
    RET=$?

    echo -n "TEST: $DESC → "

    if [[ "$EXPECT" == "OK" && $RET -eq 0 ]]; then
        echo "PASS"
        OK=$((OK+1))
    elif [[ "$EXPECT" == "FAIL" && $RET -ne 0 ]]; then
        echo "PASS"
        OK=$((OK+1))
    else
        echo "FAIL"
        FAIL=$((FAIL+1))
    fi
}

# 1 – Missing required args
run_test "Missing -s"        "$BIN -f blocked.txt"          "FAIL"

# 2 – Missing -f
run_test "Missing -f"        "$BIN -s 1.1.1.1"               "FAIL"

# 3 – Unknown argument
run_test "Unknown argument"  "$BIN -s 1.1.1.1 -f blocked.txt -X" "FAIL"

# 4 – Invalid port number
run_test "Invalid port"      "$BIN -s 1.1.1.1 -f blocked.txt -p x" "FAIL"

# 5 – Valid invocation
run_test "Valid args"        "$BIN -s 1.1.1.1 -f blocked.txt -p 8053" "OK"

echo "Main: $OK passed, $FAIL failed."
[[ $FAIL -eq 0 ]]
