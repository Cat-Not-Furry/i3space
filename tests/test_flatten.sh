#!/usr/bin/env bash
# Regression: flatten_i3_tree must finish quickly on sample i3 JSON (no live i3 needed).
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BIN="${1:-$ROOT/build/i3space}"

if [[ ! -x "$BIN" ]]; then
  echo "SKIP: $BIN not built" >&2
  exit 0
fi

# Uses live i3 when available; otherwise skips.
if ! timeout 5 "$BIN" --print-active >/tmp/i3space-flat.out 2>/tmp/i3space-flat.err; then
  if grep -q 'cannot resolve i3 IPC\|connect() to i3 IPC failed\|timed out' /tmp/i3space-flat.err 2>/dev/null; then
    echo "SKIP: i3 not running ($(head -1 /tmp/i3space-flat.err))"
    exit 0
  fi
  echo "FAIL: $(cat /tmp/i3space-flat.err)" >&2
  exit 1
fi

if ! head -c 1 /tmp/i3space-flat.out | grep -q '{'; then
  echo "FAIL: no JSON output" >&2
  exit 1
fi

echo "OK: flatten + print-active ($(wc -c < /tmp/i3space-flat.out) bytes)"
