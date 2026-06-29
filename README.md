# Network Flow Collector & Query Service

A backend system for collecting, normalizing, storing, and querying network flow records from multiple sources.

## Current Features

- REST ingest API for normalized flow records
- C++ Qt collector engine
- Zeek JSON file collector
- Suricata EVE JSON file collector
- NetFlow v5 UDP collector and binary parser
- NetFlow v9 template-based parser MVP
- Redpanda-based ingestion queue
- ClickHouse storage with partitioned flow table
- FastAPI query API and Swagger documentation
- Health and metrics endpoints
- Benchmark scripts

## Architecture

```text
C++ Qt Collectors / REST Clients
        ↓
REST Ingest API
        ↓
Redpanda
        ↓
Ingestion Worker
        ↓
ClickHouse
        ↓
FastAPI Query API
```

## Services

- Redpanda: Kafka-compatible durable queue
- ClickHouse: analytical storage for flow records
- FastAPI: query API, REST ingest API, health and metrics
- C++ Qt Collector: Zeek, Suricata, NetFlow v5, and NetFlow v9 collectors

## Quick Start

Start backend services:

```bash
sudo docker compose up -d --build
```

Check health:

```bash
curl http://localhost:8000/health
curl http://localhost:8000/metrics
```

Run ingestion worker:

```bash
sudo docker compose exec api python -m app.worker
```

Build C++ Qt collector:

```bash
cd cpp-collector
cmake -S . -B build
cmake --build build
```

## Demo Commands

### Zeek

```bash
cd cpp-collector
./build/flow_collector --mode zeek --file ../samples/zeek_conn.log
```

### Suricata

```bash
cd cpp-collector
./build/flow_collector --mode suricata --file ../samples/suricata_eve.json
```

### NetFlow v5

Terminal 1:

```bash
cd cpp-collector
./build/flow_collector --mode netflow-v5 --port 2055
```

Terminal 2:

```bash
python3 scripts/send_netflow_v5.py --count 2
```

### NetFlow v9 MVP

Terminal 1:

```bash
cd cpp-collector
./build/flow_collector --mode netflow-v9 --port 9996
```

Terminal 2:

```bash
python3 scripts/send_netflow_v9.py --port 9996 --count 2
```

NetFlow v9 is implemented as an MVP template-based parser for common IPv4 flow fields, not as a full NetFlow v9 implementation.

## Query API

```bash
curl http://localhost:8000/api/v1/flows
curl http://localhost:8000/api/v1/stats/top-talkers
curl http://localhost:8000/api/v1/stats/top-ports
curl http://localhost:8000/metrics
```

Swagger/OpenAPI:

```text
http://localhost:8000/docs
```

## Benchmark

Ingest benchmark:

```bash
python3 scripts/benchmark_ingest.py --total 10000 --batch-size 1000 --workers 4
```

Result:

```text
Accepted flows : 10000
Elapsed seconds: 0.707
Ingest rate    : 14148.62 flows/sec
```

Query benchmark:

```bash
python3 scripts/benchmark_query.py --rounds 10
```

Average query latency:

| Endpoint | Average Latency |
|---|---:|
| `/api/v1/flows?limit=100` | 0.0237s |
| `/api/v1/stats/top-talkers` | 0.0229s |
| `/api/v1/stats/top-ports` | 0.0271s |
| `/metrics` | 0.0201s |

## Documentation

- `docs/ARCHITECTURE.md`
- `docs/DEMO.md`
- `docs/COLLECTORS.md`
- `docs/API.md`
- `docs/BENCHMARK.md`
- `docs/LIMITATIONS.md`

## Notes

This project uses an at-least-once ingestion model. The worker commits queue offsets only after successful ClickHouse insertion.

NetFlow v9 support is implemented as an MVP template-based parser for common IPv4 flow fields, including `IPV4_SRC_ADDR`, `IPV4_DST_ADDR`, `L4_SRC_PORT`, `L4_DST_PORT`, `PROTOCOL`, `IN_BYTES`, `IN_PKTS`, `TCP_FLAGS`, `FIRST_SWITCHED`, and `LAST_SWITCHED`.
