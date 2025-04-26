#!/usr/bin/env bash

set -euo pipefail

declare -A _TEST_CASES
declare -i _PORT

_PORT=8080
_SERVER=$(hostname).com

_TEST_CASES=(
    ['/']='{"handlers":["/","/movie","/movies"]}'
    ['/movies']='{"handler":"MOVIES HANDLER"}'
    ['/movie']='{"handler":"MOVIE HANDLER"}'
    ['/nonexistent']="{\"error\":\"Not Found '/nonexistent'\"}"
    ["/$(python3 -c 'print("a" * 64)')"]="{\"error\":\"Not Found '/$(python3 -c 'print(("a" * 60) + "...")')'\"}"
)

for _TC in "${!_TEST_CASES[@]}"; do
    if curl -sS "https://${_SERVER}:${_PORT}${_TC}" \
        | jq --raw-output --indent 0 \
        | diff <(echo "${_TEST_CASES[$_TC]}") - ; then
        printf '%-67s test passed\n' "[$_TC]"
    else
        printf '[%s] test failed\n' "$_TC"
    fi
done
