import argparse
import json
import time
import urllib.request
from concurrent.futures import ThreadPoolExecutor, as_completed


def make_flow(i: int):
    return {
        "event_id": f"bench-{int(time.time())}-{i}",
        "source_type": "benchmark",
        "sensor_id": "bench-client",
        "src_ip": f"10.20.0.{(i % 250) + 1}",
        "dst_ip": f"8.8.8.{((i // 250) % 250) + 1}",
        "src_port": 10000 + (i % 50000),
        "dst_port": [53, 80, 443, 8080][i % 4],
        "protocol": ["UDP", "TCP"][i % 2],
        "bytes": 100 + (i % 10000),
        "packets": 1 + (i % 100),
        "start_time": "2026-06-21T10:00:00",
        "end_time": "2026-06-21T10:00:01",
        "duration": 1.0,
        "app_protocol": ["dns", "http", "tls", "unknown"][i % 4],
        "tcp_flags": "ACK" if i % 2 else "",
        "raw": {"generator": "benchmark_ingest", "index": i},
    }


def post_batch(url: str, flows):
    body = json.dumps(flows).encode("utf-8")
    req = urllib.request.Request(
        url,
        data=body,
        headers={"Content-Type": "application/json"},
        method="POST",
    )
    with urllib.request.urlopen(req, timeout=30) as res:
        res.read()
        if res.status < 200 or res.status >= 300:
            raise RuntimeError(f"HTTP {res.status}")
        return len(flows)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--url", default="http://localhost:8000/api/v1/ingest/flows")
    parser.add_argument("--total", type=int, default=10000)
    parser.add_argument("--batch-size", type=int, default=1000)
    parser.add_argument("--workers", type=int, default=4)
    args = parser.parse_args()

    batches = []
    batch = []

    for i in range(args.total):
        batch.append(make_flow(i))
        if len(batch) >= args.batch_size:
            batches.append(batch)
            batch = []

    if batch:
        batches.append(batch)

    start = time.time()
    accepted = 0

    with ThreadPoolExecutor(max_workers=args.workers) as executor:
        futures = [executor.submit(post_batch, args.url, b) for b in batches]
        for f in as_completed(futures):
            accepted += f.result()

    elapsed = time.time() - start
    rate = accepted / elapsed if elapsed > 0 else 0

    print(f"Accepted flows : {accepted}")
    print(f"Elapsed seconds: {elapsed:.3f}")
    print(f"Ingest rate    : {rate:.2f} flows/sec")


if __name__ == "__main__":
    main()
