#!/usr/bin/env bash

set -euo pipefail

declare -A _TEST_CASES
declare -i _PORT

_PORT=8080
_SERVER="$(hostname)"

_TEST_CASES=(
    ['/']=200
    ['/movies']=400
    ['/nonexistent']=404
    ["/$(python3 -c 'print("a" * 64)')"]=400
    ["/movies?search_pattern=The&search_type=startswith&search_by=title"]=200
    ["/movies?search_pattern=father&search_type=endswith&search_by=title"]=200
    ["/movies?search_pattern=The%20Godfather&search_type=exact&search_by=title"]=200
    ["/movies?search_pattern=Godfather&search_type=contains&search_by=title"]=200
)

_PASS_COUNT=0
_FAIL_COUNT=0

_check_case() {
    local tc="$1"
    local expected="$2"
    local url="https://${_SERVER}:${_PORT}${tc}"
    local resp_code
    local elapsed
    DEBUG=${DEBUG:-0}

    read -r resp_code elapsed < <(
        curl --cacert pki/mycrib.pem -sS -o /dev/null \
            -w "%{response_code} %{time_total}\n" \
            "$url"
    )

    if [ "$resp_code" -eq "$expected" ]; then
        printf '%s \033[0;32m[HTTP %s] passed (%.3fs)\033[0m\n' \
            "[$tc]" "$resp_code" "$elapsed"
        _PASS_COUNT=$((_PASS_COUNT + 1))
        if [ -n "$DEBUG" ] && [ "$DEBUG" = '1' ]; then
            curl --cacert pki/mycrib.pem -sS "$url" | jq
        fi
    else
        printf '%s \033[0;31m[HTTP %s] failed (%.3fs)\033[0m\n' \
            "[$tc]" "$resp_code" "$elapsed"
        _FAIL_COUNT=$((_FAIL_COUNT + 1))
    fi
}

BUILDDIR=build
(cd $BUILDDIR && cmake -DCMAKE_BUILD_TYPE=Release .. && make)

cd $BUILDDIR || exit

_ASAN_OPTIONS='log_path=stdout'
case $(uname) in
    Linux)
        _ASAN_OPTIONS+=',detect_leaks=1'
        ;;
    *)
        ;;
esac
./mycrib >sanitize.log 2>&1 &
PID=$!
cd .. || exit

while ! nc -zvw 1 "$(hostname)" 8080 >/dev/null 2>&1; do
    sleep 0.5
done

printf '[INFO] server is running (pid=%d)\n' $PID
for _TC in "${!_TEST_CASES[@]}"; do
    _check_case "$_TC" "${_TEST_CASES[$_TC]}"
done

echo
if [ "$_FAIL_COUNT" -eq 0 ]; then
    printf '\033[0;32mAll %d tests passed! ✅\033[0m\n' "$_PASS_COUNT"
else
    printf '\033[0;33mSummary: %d passed, %d failed ⚠️\033[0m\n' \
        "$_PASS_COUNT" "$_FAIL_COUNT"
fi

printf '[INFO] stopping server (pid=%d)\n' $PID

if ! kill -TERM $PID || ! wait $PID; then
    echo '[ERROR] failed to stop server'
fi

unset _FAIL_COUNT _PASS_COUNT _PORT _SERVER _TC _TEST_CASES
