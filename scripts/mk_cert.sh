#!/usr/bin/env bash

set -euo pipefail

declare -i _BITS _DAYS

_PROJECT_HOME="$HOME/Programming/projects/mycrib"
_OUT="$_PROJECT_HOME/pki"

_gen_key ()
{
    # $1 => number of bits

    printf 'Generating RSA key, bits=%d... ' "$1"
    openssl genrsa -out "$_OUT/mycrib.key"
    echo 'Done'
}

_gen_ss_cert ()
{
    # $1 => number of days

    printf 'Generating self-signed certificate... '
    openssl req -days "$1" -out "$_OUT/mycrib.pem" -new -x509 -key "$_OUT/mycrib.key"
    echo 'Done'
}

if ! [ -d "$_OUT" ]; then
    mkdir -p "$_OUT"
fi

_BITS=4096
if [ -f "$_OUT/mycrib.key" ]; then
    read -p 'Key exists, overwrite (y/n)? ' -r -n 1 _PERM; echo
    [ "${_PERM@L}" = 'y' ] && _gen_key $_BITS
else
    _gen_key $_BITS
fi

_DAYS=365
if [ -f "$_OUT/mycrib.pem" ]; then
    read -p 'Certificate exists, overwrite (y/n)? ' -r -n 1 _PERM; echo
    [ "${_PERM@L}" = 'y' ] && _gen_ss_cert $_DAYS
else
    _gen_ss_cert $_DAYS
fi

unset _BITS _OUT _PERM _PROJECT_HOME
