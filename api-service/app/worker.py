import json
import signal
import sys
import time
from typing import List

from confluent_kafka import Consumer, KafkaException, KafkaError

from .config import KAFKA_BOOTSTRAP_SERVERS, FLOW_TOPIC, DEAD_LETTER_TOPIC
from .insert import insert_flows_batch
from .producer import get_producer


BATCH_SIZE = 1000
POLL_TIMEOUT = 1.0
MAX_RETRIES = 5

running = True


def log_json(level: str, event: str, **kwargs):
    payload = {
        "level": level,
        "event": event,
        **kwargs,
    }
    print(json.dumps(payload), flush=True)


def handle_signal(sig, frame):
    global running
    running = False
    log_json("info", "worker_shutdown_signal", signal=sig)


def create_consumer() -> Consumer:
    consumer = Consumer({
        "bootstrap.servers": KAFKA_BOOTSTRAP_SERVERS,
        "group.id": "flowlog-ingestion-worker",
        "auto.offset.reset": "earliest",
        "enable.auto.commit": False,
        "client.id": "flowlog-worker-1",
    })

    consumer.subscribe([FLOW_TOPIC])
    return consumer


def send_dead_letter(flow: dict, error: str):
    producer = get_producer()
    payload = {
        "error": error,
        "flow": flow,
    }

    producer.produce(
        DEAD_LETTER_TOPIC,
        key=flow.get("event_id", ""),
        value=json.dumps(payload).encode("utf-8"),
    )
    producer.flush()


def insert_with_retry(flows: List[dict]) -> bool:
    delay = 1

    for attempt in range(1, MAX_RETRIES + 1):
        try:
            inserted = insert_flows_batch(flows)
            log_json(
                "info",
                "batch_insert_success",
                inserted=inserted,
                attempt=attempt,
            )
            return True

        except Exception as e:
            log_json(
                "error",
                "batch_insert_failed",
                attempt=attempt,
                error=str(e),
                retry_delay=delay,
            )

            if attempt == MAX_RETRIES:
                for flow in flows:
                    send_dead_letter(flow, str(e))
                return False

            time.sleep(delay)
            delay *= 2

    return False


def consume_loop():
    consumer = create_consumer()

    batch: list[dict] = []
    messages = []

    log_json("info", "worker_started", topic=FLOW_TOPIC)

    try:
        while running:
            msg = consumer.poll(POLL_TIMEOUT)

            if msg is None:
                if batch:
                    ok = insert_with_retry(batch)
                    if ok:
                        consumer.commit(message=messages[-1], asynchronous=False)
                        log_json("info", "offset_committed", batch_size=len(batch))
                    batch.clear()
                    messages.clear()
                continue

            if msg.error():
                if msg.error().code() == KafkaError._PARTITION_EOF:
                    continue
                raise KafkaException(msg.error())

            try:
                flow = json.loads(msg.value().decode("utf-8"))
                batch.append(flow)
                messages.append(msg)

            except Exception as e:
                log_json("error", "message_decode_failed", error=str(e))
                continue

            if len(batch) >= BATCH_SIZE:
                ok = insert_with_retry(batch)

                if ok:
                    consumer.commit(message=messages[-1], asynchronous=False)
                    log_json("info", "offset_committed", batch_size=len(batch))

                batch.clear()
                messages.clear()

    finally:
        if batch:
            ok = insert_with_retry(batch)
            if ok and messages:
                consumer.commit(message=messages[-1], asynchronous=False)

        consumer.close()
        log_json("info", "worker_stopped")


if __name__ == "__main__":
    signal.signal(signal.SIGINT, handle_signal)
    signal.signal(signal.SIGTERM, handle_signal)

    consume_loop()

