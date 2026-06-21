import json
from datetime import datetime
from typing import Any

from .clickhouse import get_client


def parse_datetime(value: Any) -> datetime:
    if isinstance(value, datetime):
        return value

    if isinstance(value, str):
        return datetime.fromisoformat(value.replace("Z", "+00:00")).replace(tzinfo=None)

    raise ValueError(f"Invalid datetime value: {value}")


def normalize_row(flow: dict) -> tuple:
    raw_value = flow.get("raw", {})

    return (
        flow.get("event_id", ""),
        flow.get("source_type", ""),
        flow.get("sensor_id", ""),

        flow.get("src_ip"),
        flow.get("dst_ip"),
        int(flow.get("src_port", 0)),
        int(flow.get("dst_port", 0)),
        flow.get("protocol", "").upper(),

        int(flow.get("bytes", 0)),
        int(flow.get("packets", 0)),

        parse_datetime(flow.get("start_time")),
        parse_datetime(flow.get("end_time")),
        float(flow.get("duration", 0.0)),

        flow.get("app_protocol", ""),
        flow.get("tcp_flags", ""),

        json.dumps(raw_value),
    )


def insert_flows_batch(flows: list[dict]) -> int:
    if not flows:
        return 0

    client = get_client()

    rows = [normalize_row(flow) for flow in flows]

    client.insert(
        "flows",
        rows,
        column_names=[
            "event_id",
            "source_type",
            "sensor_id",
            "src_ip",
            "dst_ip",
            "src_port",
            "dst_port",
            "protocol",
            "bytes",
            "packets",
            "start_time",
            "end_time",
            "duration",
            "app_protocol",
            "tcp_flags",
            "raw",
        ],
    )

    return len(rows)
