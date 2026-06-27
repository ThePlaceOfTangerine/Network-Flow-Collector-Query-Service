# Network Flow Collector & Query Service - Architecture

## 1. Overview

This project implements a backend system for collecting, normalizing, storing, and querying network flow records from multiple sources.

The system supports:

* REST ingest API
* Zeek JSON file collector
* Suricata EVE JSON file collector
* NetFlow v5 UDP collector
* NetFlow v9 template-based MVP parser
* Redpanda-based ingestion queue
* ClickHouse analytical storage
* FastAPI query API
* Health and metrics endpoints
* Benchmark scripts

## 2. High-Level Architecture

```text
+-------------------------+
| C++ Qt Collector Engine |
| - Zeek JSON             |
| - Suricata EVE JSON     |
| - NetFlow v5 UDP        |
| - NetFlow v9 UDP MVP    |
+-----------+-------------+
            |
            v
+-------------------------+
| REST Ingest API         |
| POST /api/v1/ingest/flows |
+-----------+-------------+
            |
            v
+-------------------------+
| Redpanda Queue          |
| topic: flows.normalized |
+-----------+-------------+
            |
            v
+-------------------------+
| Ingestion Worker        |
| batch insert            |
| retry/backoff           |
| manual offset commit    |
+-----------+-------------+
            |
            v
+-------------------------+
| ClickHouse Storage      |
| flows table             |
| materialized views      |
+-----------+-------------+
            |
            v
+-------------------------+
| FastAPI Query API       |
| /flows                  |
| /stats/top-talkers      |
| /stats/top-ports        |
| /metrics                |
+-------------------------+
```

## 3. Main Components

### 3.1 C++ Qt Collector Engine

The collector engine is implemented in C++ using Qt Core and Qt Network.

It provides a plugin-style structure:

* `ICollector`: base collector interface
* `ISink`: output sink interface
* `NormalizedFlow`: unified flow model
* `HttpSink`: sends normalized flows to the REST ingest API

Supported collector modes:

```bash
./flow_collector --mode fake
./flow_collector --mode zeek --file ../samples/zeek_conn.log
./flow_collector --mode suricata --file ../samples/suricata_eve.json
./flow_collector --mode netflow-v5 --port 2055
./flow_collector --mode netflow-v9 --port 9996
```

### 3.2 REST Ingest API

The REST ingest API receives normalized flow records and publishes them to Redpanda.

Endpoint:

```http
POST /api/v1/ingest/flows
```

This endpoint is used by:

* C++ Qt collectors
* benchmark scripts
* integration tests
* manual REST clients

### 3.3 Redpanda Queue

Redpanda is used as a Kafka-compatible durable queue.

Main topic:

```text
flows.normalized
```

Purpose:

* Decouple collectors/API from ClickHouse
* Buffer bursts of flow records
* Enable at-least-once ingestion
* Allow multiple collector instances

### 3.4 Ingestion Worker

The worker consumes records from Redpanda and inserts them into ClickHouse in batches.

Reliability behavior:

* Messages are inserted into ClickHouse first
* Offsets are committed only after successful insert
* Failed batches are retried with exponential backoff
* Failed records can be sent to a dead-letter topic

This provides at-least-once delivery semantics.

### 3.5 ClickHouse Storage

ClickHouse stores normalized flow records in a MergeTree table.

The storage is optimized for:

* Time-based partitioning
* Source/destination IP queries
* Protocol/port filtering
* Aggregation queries

Materialized views are used for:

* Top talkers
* Top ports

### 3.6 Query API

The FastAPI service exposes:

```http
GET /health
GET /metrics
GET /api/v1/flows
GET /api/v1/stats/top-talkers
GET /api/v1/stats/top-ports
```

Swagger/OpenAPI documentation is available at:

```http
GET /docs
```

## 4. Normalized Flow Schema

All collectors convert source-specific records into a common schema:

| Field          | Description                                         |
| -------------- | --------------------------------------------------- |
| `event_id`     | Unique flow event ID                                |
| `source_type`  | Input source, e.g. `zeek`, `suricata`, `netflow_v5` |
| `sensor_id`    | Collector/sensor identifier                         |
| `src_ip`       | Source IP address                                   |
| `dst_ip`       | Destination IP address                              |
| `src_port`     | Source port                                         |
| `dst_port`     | Destination port                                    |
| `protocol`     | Transport protocol                                  |
| `bytes`        | Total bytes                                         |
| `packets`      | Total packets                                       |
| `start_time`   | Flow start time                                     |
| `end_time`     | Flow end time                                       |
| `duration`     | Flow duration                                       |
| `app_protocol` | Application protocol if available                   |
| `tcp_flags`    | TCP flags if available                              |
| `raw`          | Original raw record or metadata                     |

## 5. Supported Sources

### 5.1 Zeek JSON

The Zeek collector maps Zeek `conn.log` JSON fields into the normalized flow schema.

Examples:

| Zeek Field  | Normalized Field |
| ----------- | ---------------- |
| `id.orig_h` | `src_ip`         |
| `id.resp_h` | `dst_ip`         |
| `id.orig_p` | `src_port`       |
| `id.resp_p` | `dst_port`       |
| `proto`     | `protocol`       |
| `service`   | `app_protocol`   |

### 5.2 Suricata EVE JSON

The Suricata collector maps Suricata flow events where:

```json
{
  "event_type": "flow"
}
```

Examples:

| Suricata Field                              | Normalized Field |
| ------------------------------------------- | ---------------- |
| `src_ip`                                    | `src_ip`         |
| `dest_ip`                                   | `dst_ip`         |
| `src_port`                                  | `src_port`       |
| `dest_port`                                 | `dst_port`       |
| `flow.bytes_toserver + flow.bytes_toclient` | `bytes`          |

### 5.3 NetFlow v5

The NetFlow v5 collector listens on UDP and parses fixed-size binary records.

Supported fields include:

* Source IP
* Destination IP
* Source port
* Destination port
* Packets
* Bytes
* Protocol
* TCP flags

### 5.4 NetFlow v9 MVP

The NetFlow v9 parser supports a template-based MVP for common IPv4 flow fields:

* `IPV4_SRC_ADDR`
* `IPV4_DST_ADDR`
* `L4_SRC_PORT`
* `L4_DST_PORT`
* `PROTOCOL`
* `IN_BYTES`
* `IN_PKTS`
* `TCP_FLAGS`
* `FIRST_SWITCHED`
* `LAST_SWITCHED`

This is not a full NetFlow v9 implementation, but it demonstrates template-based parsing.

## 6. Delivery Semantics

The system uses an at-least-once delivery model.

A message is considered processed only after the worker successfully inserts it into ClickHouse and commits the queue offset.

This means:

* Data loss is reduced
* Duplicate records are possible after retries or crashes
* Downstream deduplication can be added later using `event_id`

## 7. Performance

Benchmark result:

```text
Accepted flows : 10000
Elapsed seconds: 0.707
Ingest rate    : 14148.62 flows/sec
```

The system exceeded the target requirement of 5,000 flows/sec.

Average query latency:

| Endpoint                    | Average Latency |
| --------------------------- | --------------: |
| `/api/v1/flows?limit=100`   |         0.0237s |
| `/api/v1/stats/top-talkers` |         0.0229s |
| `/api/v1/stats/top-ports`   |         0.0271s |
| `/metrics`                  |         0.0201s |
