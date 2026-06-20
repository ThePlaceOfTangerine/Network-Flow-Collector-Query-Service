
# FlowLog System

Backend system for collecting, normalizing, storing and querying network flow logs.

## Services

- Redpanda: Kafka-compatible durable queue
- ClickHouse: analytical storage for flow records
- FastAPI: query API, REST ingest API, health and metrics

## Run

```bash
docker compose up -d --build
