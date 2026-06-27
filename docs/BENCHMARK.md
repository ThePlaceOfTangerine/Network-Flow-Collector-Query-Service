# Benchmark Report

## Environment

- Machine: Dell Inspiron 5593
- OS: Ubuntu 20.04
- Deployment: Docker Compose
- Queue: Redpanda
- Storage: ClickHouse
- API: FastAPI
- Collector: C++ Qt

## Ingest Benchmark

Command:

```bash
python3 scripts/benchmark_ingest.py --total 10000 --batch-size 1000 --workers 4


Result:

Accepted flows : 10000
Elapsed seconds: 0.707
Ingest rate    : 14148.62 flows/sec

Notes

This benchmark measures the REST ingest API accepted throughput into the Redpanda queue.

The full pipeline is:

REST Ingest API
    ↓
Redpanda topic: flows.normalized
    ↓
Ingestion Worker
    ↓
ClickHouse
