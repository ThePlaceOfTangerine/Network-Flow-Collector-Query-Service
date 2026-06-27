import socket
import struct
import time
import argparse
import ipaddress


def ip_to_int(ip: str) -> int:
    return int(ipaddress.IPv4Address(ip))


def pad4(data: bytes) -> bytes:
    pad_len = (4 - (len(data) % 4)) % 4
    return data + (b"\x00" * pad_len)


def build_netflow_v9_packet(count: int = 2) -> bytes:
    version = 9
    sys_uptime = 123456
    unix_secs = int(time.time())
    sequence_number = 1
    source_id = 256

    template_id = 256

    fields = [
        (8, 4),    # IPV4_SRC_ADDR
        (12, 4),   # IPV4_DST_ADDR
        (7, 2),    # L4_SRC_PORT
        (11, 2),   # L4_DST_PORT
        (4, 1),    # PROTOCOL
        (1, 4),    # IN_BYTES
        (2, 4),    # IN_PKTS
        (6, 1),    # TCP_FLAGS
        (22, 4),   # FIRST_SWITCHED
        (21, 4),   # LAST_SWITCHED
    ]

    template_record = struct.pack("!HH", template_id, len(fields))

    for field_type, field_length in fields:
        template_record += struct.pack("!HH", field_type, field_length)

    template_flowset_length = 4 + len(template_record)
    template_flowset = struct.pack("!HH", 0, template_flowset_length) + template_record

    sample_flows = [
        {
            "src": "192.168.30.10",
            "dst": "8.8.8.8",
            "src_port": 51514,
            "dst_port": 53,
            "protocol": 17,
            "bytes": 300,
            "packets": 4,
            "tcp_flags": 0,
            "first": 1000,
            "last": 2000,
        },
        {
            "src": "192.168.30.20",
            "dst": "1.1.1.1",
            "src_port": 44321,
            "dst_port": 443,
            "protocol": 6,
            "bytes": 7000,
            "packets": 15,
            "tcp_flags": 24,
            "first": 2000,
            "last": 3500,
        },
    ]

    records = b""

    for i in range(count):
        flow = sample_flows[i % len(sample_flows)]

        record = struct.pack(
            "!IIHHB I I B I I",
            ip_to_int(flow["src"]),
            ip_to_int(flow["dst"]),
            flow["src_port"],
            flow["dst_port"],
            flow["protocol"],
            flow["bytes"],
            flow["packets"],
            flow["tcp_flags"],
            flow["first"],
            flow["last"],
        )

        records += record

    data_body = pad4(records)
    data_flowset_length = 4 + len(data_body)
    data_flowset = struct.pack("!HH", template_id, data_flowset_length) + data_body

    header = struct.pack(
        "!HHIIII",
        version,
        count,
        sys_uptime,
        unix_secs,
        sequence_number,
        source_id,
    )

    return header + template_flowset + data_flowset


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--port", type=int, default=9996)
    parser.add_argument("--count", type=int, default=2)
    args = parser.parse_args()

    packet = build_netflow_v9_packet(args.count)

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.sendto(packet, (args.host, args.port))
    sock.close()

    print(
        f"Sent NetFlow v9 packet to {args.host}:{args.port}, "
        f"flows={args.count}, bytes={len(packet)}"
    )


if __name__ == "__main__":
    main()
