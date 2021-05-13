#!/bin/bash

# Arguments: $1: Interface ('grep'-regexp).

# Static list of types (from `ip link help`):
TYPES=(ngmwan)

iface="$1"

for type in "${TYPES[@]}"; do
  ip link show type "${type}" | grep -E '^[0-9]+:' | cut -d ':' -f 2 | sed 's|^[[:space:]]*||' | while read _if; do
    echo "${_if}:${type}"
  done | grep "^${iface}"
done
