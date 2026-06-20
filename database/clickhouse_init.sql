CREATE DATABASE IF NOT EXISTS flowlog;

CREATE TABLE IF NOT EXISTS flowlog.flows
(
    event_id String,

    source_type LowCardinality(String),
    sensor_id LowCardinality(String),

    src_ip IPv4,
    dst_ip IPv4,
    src_port UInt16,
    dst_port UInt16,
    protocol LowCardinality(String),

    bytes UInt64,
    packets UInt64,

    start_time DateTime64(3),
    end_time DateTime64(3),
    duration Float64,

    app_protocol LowCardinality(String),
    tcp_flags LowCardinality(String),

    raw String,
    created_at DateTime DEFAULT now()
)
ENGINE = MergeTree
PARTITION BY toYYYYMMDD(start_time)
ORDER BY (start_time, src_ip, dst_ip, dst_port, protocol)
SETTINGS index_granularity = 8192;


CREATE MATERIALIZED VIEW IF NOT EXISTS flowlog.mv_top_talkers_hourly
ENGINE = SummingMergeTree
PARTITION BY toYYYYMMDD(hour)
ORDER BY (hour, src_ip)
AS
SELECT
    toStartOfHour(start_time) AS hour,
    src_ip,
    count() AS flow_count,
    sum(bytes) AS total_bytes,
    sum(packets) AS total_packets
FROM flowlog.flows
GROUP BY hour, src_ip;


CREATE MATERIALIZED VIEW IF NOT EXISTS flowlog.mv_top_ports_hourly
ENGINE = SummingMergeTree
PARTITION BY toYYYYMMDD(hour)
ORDER BY (hour, dst_port)
AS
SELECT
    toStartOfHour(start_time) AS hour,
    dst_port,
    count() AS flow_count,
    sum(bytes) AS total_bytes
FROM flowlog.flows
GROUP BY hour, dst_port;


CREATE TABLE IF NOT EXISTS flowlog.ingest_metrics
(
    metric_name String,
    metric_value UInt64,
    updated_at DateTime DEFAULT now()
)
ENGINE = MergeTree
ORDER BY metric_name;

