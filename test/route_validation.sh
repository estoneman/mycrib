#!/usr/bin/env bash

set -euo pipefail

declare -A _TEST_CASES
declare -i _PORT

_PORT=8080
_SERVER=$(hostname).com

_TEST_CASES=(
    ['/']=200
    ['/movies']=400
    ['/nonexistent']=404
    ["/$(python3 -c 'print("a" * 64)')"]=400
    ["/movies?search_pattern=The&search_type=startswith"]=200
)

_PASS_COUNT=0
_FAIL_COUNT=0

_check_case() {
    local tc="$1"
    local expected="$2"
    local url="https://${_SERVER}:${_PORT}${tc}"
    local resp_code
    local elapsed

    read -r resp_code elapsed < <(
        curl -sS -o /dev/null \
            -w "%{response_code} %{time_total}\n" \
            "$url"
    )

    if [ "$resp_code" -eq "$expected" ]; then
        printf '%s \033[0;32m[HTTP %s] passed (%.3fs)\033[0m\n' \
            "[$tc]" "$resp_code" "$elapsed"
        _PASS_COUNT=$((_PASS_COUNT + 1))
    else
        printf '%s \033[0;31m[HTTP %s] failed (%.3fs)\033[0m\n' \
            "[$tc]" "$resp_code" "$elapsed"
        _FAIL_COUNT=$((_FAIL_COUNT + 1))
    fi
}

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

unset _FAIL_COUNT _PASS_COUNT _PORT _SERVER _TC _TEST_CASES
