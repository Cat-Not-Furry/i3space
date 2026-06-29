#!/usr/bin/env bash
set -euo pipefail
BIN="${1:-./build/i3space}"
"$BIN" --version | grep -q 'i3space 0.2.0'
echo "OK: version"
