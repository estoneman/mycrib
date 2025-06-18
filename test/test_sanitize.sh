#!/usr/bin/env bash

set -euo pipefail

BUILDDIR=build
(cd $BUILDDIR && cmake -DCMAKE_BUILD_TYPE=Debug .. && make)

cd $BUILDDIR || exit

_ASAN_OPTIONS='log_path=stdout'
case $(uname) in
    Linux)
        _ASAN_OPTIONS+=',detect_leaks=1'
        ;;
    *)
        ;;
esac
ASAN_OPTIONS=$_ASAN_OPTIONS \
UBSAN_OPTIONS=log_path=stdout,halt_on_error=1 \
    ./mycrib >sanitize.log 2>&1 &
PID=$!

cd .. || exit

while ! nc -zvw 1 mac-terminull.com 8080 >/dev/null 2>&1; do
    sleep 0.5
done

printf '[INFO] server is running (pid=%d)\n' $PID

./test/test_route.sh

# server will still be running if sanitization checks passed
if ! pgrep mycrib >/dev/null 2>&1; then
    printf '[ERROR] sanitization checks failed, see %s/sanitize.log' $BUILDDIR
fi

printf '[INFO] stopping server (pid=%d)\n' $PID

if ! kill -TERM $PID || ! wait $PID; then
    echo '[ERROR] failed to stop server'
fi
