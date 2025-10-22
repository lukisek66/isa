#!/usr/bin/env bash
set -euo pipefail
ROOT=$(cd "$(dirname "$0")/.." && pwd)
TESTDIR="$ROOT/tests"
DNS_BIN="$ROOT/dns"
BLOCKED="$ROOT/blocked.txt"

# Ports
DNS_PORT=8053          # where dns listens for clients
MOCK_PORT=8054         # where mock resolver listens
MOCK_HOST=127.0.0.1

# Start mock resolver
echo "[RUN] Starting mock resolver..."
python3 "$TESTDIR/mock_resolver.py" "$MOCK_HOST" "$MOCK_PORT" &>/tmp/mock_resolver.log &
MOCK_PID=$!
sleep 0.5

# Build
echo "[RUN] Building project..."
(cd "$ROOT" && make) >/dev/null

# Start dns pointing to mock resolver
echo "[RUN] Starting dns server (forward -> ${MOCK_HOST}:${MOCK_PORT})..."
"$DNS_BIN" -s "$MOCK_HOST" -p "$MOCK_PORT" -f "$BLOCKED" -v &>/tmp/dns_server.log &
DNS_PID=$!
sleep 0.5

on_exit() {
  echo "[RUN] Stopping processes..."
  kill $DNS_PID 2>/dev/null || true
  kill $MOCK_PID 2>/dev/null || true
  wait $DNS_PID 2>/dev/null || true
  wait $MOCK_PID 2>/dev/null || true
}
trap on_exit EXIT

# List of tests (order: golden path then edge cases then cleanup)
tests=(
  T01_args_variants.sh
  T02_forward_A_ipv4.sh
  T02b_forward_A_mock_variant.sh
  T03_forward_A_ipv6.sh
  T04_refused.sh
  T05_notimp.sh
  T06_nxdomain.sh
  T07_qname_compression.py
  T08_id_restore.sh
  E10_bind_errors.sh
  E1_malformed_short.py
  E12_sigint_shutdown.sh
  E2_invalid_label.py
  E3_pointer_loop.py
  E4_large_qname.py
  E6_flood.py
  E8_resolver_unreachable.sh
  E9_resolver_diff_source.py
  CT1_timeout_cleanup.sh
  CT2_timeout_recovery.sh
)

for t in "${tests[@]}"; do
  echo; echo "=== RUNNING $t ==="
  if [[ "$t" == *.py ]]; then
    python3 "$TESTDIR/$t"
  else
    bash "$TESTDIR/$t"
  fi
  echo "=== $t OK ==="
done

echo "ALL TESTS OK"
