#!/bin/bash
set -e

echo "*** TEST MAIN ***"
bash tests/test_main.sh

echo "*** TEST FILTER ***"
python3 tests/test_filter.py

echo "*** TEST FORWARDER ***"
python3 tests/test_forwarder.py

echo "*** TEST DNS SERVER ***"
bash tests/test_dns_server.sh

echo "All tests OK."
