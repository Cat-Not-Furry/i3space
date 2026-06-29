# Performance notes — i3space

## Targets

| Metric | MVP1 | MVP2 |
|--------|------|------|
| `--print-active` p95 | < 5 ms | < 3 ms |
| `--toggle` to first frame | < 12 ms | < 8 ms |
| Peak RSS (overview) | < 8 MiB | < 10 MiB |

## How to measure

```bash
./tests/bench_parse.sh ./build/i3space 100
perf stat -r 5 ./build/i3space --print-active
# In i3 with 2+ workspaces:
perf stat -r 3 ./build/i3space --toggle
```

## Design choices for speed

- **Single-shot process** — zero idle cost
- **simdjson on-demand** — skip unused JSON fields
- **SoA workspace model** — cache-friendly loops
- **XRender rects** — no Cairo in hot draw path
- **Redraw on selection change only** — no animation loop
- **One IPC connection per toggle** — no SUBSCRIBE on hot path

## Hardware reference

Record your results here after testing:

| Machine | parse p95 | toggle | Notes |
|---------|-----------|--------|-------|
| (fill in) | | | |
