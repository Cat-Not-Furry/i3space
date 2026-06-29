#!/usr/bin/env bash
# Benchmark i3space --print-active (requires i3 running).
set -euo pipefail

BIN="${1:-./build/i3space}"
N="${2:-50}"

if ! command -v "$BIN" >/dev/null 2>&1 && [[ -x "$BIN" ]]; then
  echo "Usage: $0 [path/to/i3space] [iterations]" >&2
  exit 1
fi

if ! "$BIN" --print-active >/dev/null 2>&1; then
  echo "SKIP: i3 not running or IPC failed ($BIN --print-active)" >&2
  exit 0
fi

times=()
for ((i = 0; i < N; i++)); do
  start=$(date +%s%N)
  "$BIN" --print-active >/dev/null
  end=$(date +%s%N)
  times+=($(((end - start) / 1000000)))
done

IFS=$'\n' sorted=($(sort -n <<<"${times[*]}"))
p50="${sorted[$((N / 2))]}"
p95="${sorted[$((N * 95 / 100))]}"

echo "bench_parse: n=$N p50=${p50}ms p95=${p95}ms"
