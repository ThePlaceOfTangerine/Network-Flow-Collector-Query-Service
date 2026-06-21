import json
from datetime import datetime
from typing import Any

from confluent_kafka import Producer

from .config import KAFKA_BOOTSTRAP_SERVERS, FLOW_TOPIC


_producer: Producer | None = None


def get_producer() -> Producer:
    global _producer

    if _producer is None:
        _producer = Producer({
            "bootstrap.servers": KAFKA_BOOTSTRAP_SERVERS,
            "client.id": "flowlog-api-producer",
            "acks": "all",
        })

    return _producer


def json_serializer(obj: Any):
    if isinstance(obj, datetime):
        return obj.isoformat()
    raise TypeError(f"Object of type {type(obj)} is not JSON serializable")


def delivery_report(err, msg):
    if err is not None:
        print(json.dumps({
            "level": "error",
            "event": "produce_failed",
            "error": str(err),
        }))
    else:
        print(json.dumps({
            "level": "info",
            "event": "produce_success",
            "topic": msg.topic(),
            "partition": msg.partition(),
            "offset": msg.offset(),
        }))


def produce_flow(flow: dict):
    producer = get_producer()

    key = flow.get("event_id", "")

    producer.produce(
        FLOW_TOPIC,
        key=key,
        value=json.dumps(flow, default=json_serializer).encode("utf-8"),
        callback=delivery_report,
    )

    producer.poll(0)


def flush_producer():
    producer = get_producer()
    producer.flush()
