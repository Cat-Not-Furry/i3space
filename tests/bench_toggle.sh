#!/usr/bin/env bash
# Smoke benchmark for toggle startup (requires i3 + X11 + 2+ workspaces).
set -euo pipefail

BIN="${1:-./build/i3space}"

if [[ -z "${DISPLAY:-}" ]]; then
  echo "SKIP: no DISPLAY" >&2
  exit 0
fi

if ! "$BIN" --print-active >/dev/null 2>&1; then
  echo "SKIP: i3 IPC unavailable" >&2
  exit 0
fi

echo "bench_toggle: manual — run 'perf stat $BIN --toggle' in i3 session with 2+ workspaces"
