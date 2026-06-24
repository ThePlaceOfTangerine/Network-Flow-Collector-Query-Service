import socket
import struct
import time
import argparse
import ipaddress


def ip_to_int(ip: str) -> int:
    return int(ipaddress.IPv4Address(ip))


def build_netflow_v5_packet(count: int = 2) -> bytes:
    version = 5
    sys_uptime = 123456
    unix_secs = int(time.time())
    unix_nsecs = 0
    flow_sequence = 1
    engine_type = 0
    engine_id = 0
    sampling_interval = 0

    header = struct.pack(
        "!HHIIIIBBH",
        version,
        count,
        sys_uptime,
        unix_secs,
        unix_nsecs,
        flow_sequence,
        engine_type,
        engine_id,
        sampling_interval,
    )

    records = b""

    sample_flows = [
        {
            "src": "192.168.10.10",
            "dst": "8.8.8.8",
            "src_port": 51514,
            "dst_port": 53,
            "protocol": 17,  # UDP
            "packets": 3,
            "bytes": 240,
            "tcp_flags": 0,
        },
        {
            "src": "192.168.10.20",
            "dst": "1.1.1.1",
            "src_port": 44321,
            "dst_port": 443,
            "protocol": 6,  # TCP
            "packets": 12,
            "bytes": 5000,
            "tcp_flags": 24,  # PSH + ACK
        },
    ]

    for i in range(count):
        flow = sample_flows[i % len(sample_flows)]

        srcaddr = ip_to_int(flow["src"])
        dstaddr = ip_to_int(flow["dst"])
        nexthop = 0

        input_if = 0
        output_if = 0

        dpkts = flow["packets"]
        doctets = flow["bytes"]

        first = 1000
        last = 2000

        srcport = flow["src_port"]
        dstport = flow["dst_port"]

        pad1 = 0
        tcp_flags = flow["tcp_flags"]
        prot = flow["protocol"]
        tos = 0

        src_as = 0
        dst_as = 0
        src_mask = 0
        dst_mask = 0
        pad2 = 0

        record = struct.pack(
            "!IIIHHIIIIHHBBBBHHBBH",
            srcaddr,
            dstaddr,
            nexthop,
            input_if,
            output_if,
            dpkts,
            doctets,
            first,
            last,
            srcport,
            dstport,
            pad1,
            tcp_flags,
            prot,
            tos,
            src_as,
            dst_as,
            src_mask,
            dst_mask,
            pad2,
        )

        records += record

    return header + records


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--port", type=int, default=2055)
    parser.add_argument("--count", type=int, default=2)
    args = parser.parse_args()

    packet = build_netflow_v5_packet(args.count)

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.sendto(packet, (args.host, args.port))
    sock.close()

    print(f"Sent NetFlow v5 packet to {args.host}:{args.port}, flows={args.count}, bytes={len(packet)}")


if __name__ == "__main__":
    main()
