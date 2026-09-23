#!/usr/bin/env bash
set -e

mkdir -p /src/build_linux
cd /src/build_linux
cmake .. -DCMAKE_BUILD_TYPE=Release -DREGEX_HELPER_SMOKE_TEST=ON
cmake --build . -- -j$(nproc)

echo '--- Smoke test ---'
./regex_smoke_test

echo '--- Verify exports ---'
EXPORTS=$(nm -D /src/build_linux/RegexHelper.so | grep -cE 'GetClassNames|GetClassObject|DestroyObject|SetPlatformCapabilities')
echo "Exports found: $EXPORTS/4"
[ "$EXPORTS" -eq 4 ] || { echo '[ERROR] Not all 4 exports found'; exit 1; }
