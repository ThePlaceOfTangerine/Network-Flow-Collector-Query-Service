import argparse
import time
import urllib.request


def get(url: str):
    start = time.time()

    with urllib.request.urlopen(url, timeout=30) as response:
        body = response.read()

    elapsed = time.time() - start
    return response.status, len(body), elapsed


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--base-url", default="http://localhost:8000")
    parser.add_argument("--rounds", type=int, default=10)

    args = parser.parse_args()

    urls = [
        f"{args.base_url}/api/v1/flows?limit=100",
        f"{args.base_url}/api/v1/stats/top-talkers",
        f"{args.base_url}/api/v1/stats/top-ports",
        f"{args.base_url}/metrics",
    ]

    for url in urls:
        print()
        print(f"Benchmark URL: {url}")

        total_time = 0.0

        for i in range(args.rounds):
            status, size, elapsed = get(url)
            total_time += elapsed
            print(
                f"round={i + 1} "
                f"status={status} "
                f"size={size} "
                f"elapsed={elapsed:.4f}s"
            )

        avg = total_time / args.rounds
        print(f"Average latency: {avg:.4f}s")


if __name__ == "__main__":
    main()

