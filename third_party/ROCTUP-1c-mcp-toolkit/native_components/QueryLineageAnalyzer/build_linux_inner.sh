#!/usr/bin/env bash
set -e

mkdir -p /src/build_linux
cd /src/build_linux
cmake .. -DCMAKE_BUILD_TYPE=Release -DQUERY_LINEAGE_BUILD_TESTS=ON
cmake --build . -- -j$(nproc)

echo '--- Run native tests ---'
./test_query_lineage

echo '--- Verify exports ---'
EXPORTS=$(nm -D /src/build_linux/QueryLineageAnalyzer.so | grep -cE 'GetClassNames|GetClassObject|DestroyObject|SetPlatformCapabilities')
echo "Exports found: $EXPORTS/4"
[ "$EXPORTS" -eq 4 ] || { echo '[ERROR] Not all 4 exports found'; exit 1; }
