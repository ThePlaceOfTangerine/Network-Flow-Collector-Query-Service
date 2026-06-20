from fastapi import FastAPI
from confluent_kafka.admin import AdminClient, NewTopic

from .clickhouse import get_client, ping_clickhouse
from .config import KAFKA_BOOTSTRAP_SERVERS, FLOW_TOPIC, DEAD_LETTER_TOPIC


app = FastAPI(
    title="FlowLog Backend API",
    description="Backend query API and REST ingest layer for normalized network flow records.",
    version="0.1.0",
)


def ensure_kafka_topics():
    admin = AdminClient({"bootstrap.servers": KAFKA_BOOTSTRAP_SERVERS})

    topics = [
        NewTopic(FLOW_TOPIC, num_partitions=3, replication_factor=1),
        NewTopic(DEAD_LETTER_TOPIC, num_partitions=1, replication_factor=1),
    ]

    try:
        admin.create_topics(topics)
    except Exception:
        # Topic có thể đã tồn tại, không cần crash API.
        pass


@app.on_event("startup")
def startup_event():
    ensure_kafka_topics()


@app.get("/")
def root():
    return {
        "service": "flowlog-api",
        "status": "running",
        "docs": "/docs",
    }


@app.get("/health")
def health():
    clickhouse_ok = ping_clickhouse()

    kafka_ok = False
    try:
        admin = AdminClient({"bootstrap.servers": KAFKA_BOOTSTRAP_SERVERS})
        metadata = admin.list_topics(timeout=5)
        kafka_ok = metadata is not None
    except Exception:
        kafka_ok = False

    status = "ok" if clickhouse_ok and kafka_ok else "degraded"

    return {
        "status": status,
        "clickhouse": "ok" if clickhouse_ok else "error",
        "redpanda": "ok" if kafka_ok else "error",
    }


@app.get("/metrics")
def metrics():
    client = get_client()

    total_flows = client.query(
        "SELECT count() FROM flows"
    ).result_rows[0][0]

    parse_errors = 0
    records_processed = total_flows

    return {
        "records_processed": records_processed,
        "records_failed": parse_errors,
        "parse_error_count": parse_errors,
        "stored_flows": total_flows,
        "ingest_rate_per_sec": 0,
        "worker_lag": 0,
    }


@app.get("/api/v1/flows")
def list_flows(limit: int = 50, offset: int = 0):
    client = get_client()

    limit = min(limit, 500)

    query = """
        SELECT
            event_id,
            source_type,
            sensor_id,
            toString(src_ip) AS src_ip,
            toString(dst_ip) AS dst_ip,
            src_port,
            dst_port,
            protocol,
            bytes,
            packets,
            start_time,
            end_time,
            duration,
            app_protocol,
            tcp_flags,
            created_at
        FROM flows
        ORDER BY start_time DESC
        LIMIT %(limit)s OFFSET %(offset)s
    """

    rows = client.query(query, parameters={"limit": limit, "offset": offset})

    columns = rows.column_names
    items = [dict(zip(columns, row)) for row in rows.result_rows]

    return {
        "total": len(items),
        "limit": limit,
        "offset": offset,
        "items": items,
    }


@app.get("/api/v1/stats/top-talkers")
def top_talkers(limit: int = 10):
    client = get_client()

    query = """
        SELECT
            toString(src_ip) AS src_ip,
            sum(flow_count) AS flow_count,
            sum(total_bytes) AS total_bytes,
            sum(total_packets) AS total_packets
        FROM mv_top_talkers_hourly
        GROUP BY src_ip
        ORDER BY total_bytes DESC
        LIMIT %(limit)s
    """

    rows = client.query(query, parameters={"limit": limit})
    columns = rows.column_names
    items = [dict(zip(columns, row)) for row in rows.result_rows]

    return {"items": items}


@app.get("/api/v1/stats/top-ports")
def top_ports(limit: int = 10):
    client = get_client()

    query = """
        SELECT
            dst_port,
            sum(flow_count) AS flow_count,
            sum(total_bytes) AS total_bytes
        FROM mv_top_ports_hourly
        GROUP BY dst_port
        ORDER BY total_bytes DESC
        LIMIT %(limit)s
    """

    rows = client.query(query, parameters={"limit": limit})
    columns = rows.column_names
    items = [dict(zip(columns, row)) for row in rows.result_rows]

    return {"items": items}
