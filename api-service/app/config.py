import os


CLICKHOUSE_HOST = os.getenv("CLICKHOUSE_HOST", "localhost")
CLICKHOUSE_PORT = int(os.getenv("CLICKHOUSE_PORT", "8123"))
CLICKHOUSE_USER = os.getenv("CLICKHOUSE_USER", "flowlog")
CLICKHOUSE_PASSWORD = os.getenv("CLICKHOUSE_PASSWORD", "flowlog")
CLICKHOUSE_DATABASE = os.getenv("CLICKHOUSE_DATABASE", "flowlog")

KAFKA_BOOTSTRAP_SERVERS = os.getenv("KAFKA_BOOTSTRAP_SERVERS", "localhost:9092")
FLOW_TOPIC = os.getenv("FLOW_TOPIC", "flows.normalized")
DEAD_LETTER_TOPIC = os.getenv("DEAD_LETTER_TOPIC", "flows.dead_letter")
