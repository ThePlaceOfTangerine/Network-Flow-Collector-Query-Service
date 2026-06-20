import clickhouse_connect

from .config import (
    CLICKHOUSE_HOST,
    CLICKHOUSE_PORT,
    CLICKHOUSE_USER,
    CLICKHOUSE_PASSWORD,
    CLICKHOUSE_DATABASE,
)


def get_client():
    return clickhouse_connect.get_client(
        host=CLICKHOUSE_HOST,
        port=CLICKHOUSE_PORT,
        username=CLICKHOUSE_USER,
        password=CLICKHOUSE_PASSWORD,
        database=CLICKHOUSE_DATABASE,
    )


def ping_clickhouse() -> bool:
    try:
        client = get_client()
        result = client.query("SELECT 1").result_rows
        return result[0][0] == 1
    except Exception:
        return False
