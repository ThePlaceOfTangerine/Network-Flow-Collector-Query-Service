# Collector Field Mapping

This document describes how each collector maps source-specific traffic records into the normalized flow schema.

## 1. Normalized Flow Schema

All collectors convert records into the following common format:

| Field | Description |
|---|---|
| `event_id` | Unique flow event ID |
| `source_type` | Source type, e.g. `zeek`, `suricata`, `netflow_v5`, `netflow_v9` |
| `sensor_id` | Collector or sensor identifier |
| `src_ip` | Source IP address |
| `dst_ip` | Destination IP address |
| `src_port` | Source port |
| `dst_port` | Destination port |
| `protocol` | Transport protocol |
| `bytes` | Total number of bytes |
| `packets` | Total number of packets |
| `start_time` | Flow start time |
| `end_time` | Flow end time |
| `duration` | Flow duration in seconds |
| `app_protocol` | Application protocol if available |
| `tcp_flags` | TCP flags if available |
| `raw` | Original record or source-specific metadata |

## 2. Zeek File Collector

The Zeek collector reads Zeek `conn.log` JSON lines and maps each connection record into the normalized flow schema.

| Zeek Field | Normalized Field |
|---|---|
| `uid` | `event_id` |
| `id.orig_h` | `src_ip` |
| `id.resp_h` | `dst_ip` |
| `id.orig_p` | `src_port` |
| `id.resp_p` | `dst_port` |
| `proto` | `protocol` |
| `service` | `app_protocol` |
| `orig_bytes + resp_bytes` | `bytes` |
| `orig_pkts + resp_pkts` | `packets` |
| Raw JSON object | `raw` |

Example command:

~~~bash
cd cpp-collector
./build/flow_collector --mode zeek --file ../samples/zeek_conn.log
~~~

## 3. Suricata EVE JSON Collector

The Suricata collector reads Suricata `eve.json` records where `event_type = flow`.

| Suricata Field | Normalized Field |
|---|---|
| `flow_id` | `event_id` |
| `src_ip` | `src_ip` |
| `dest_ip` | `dst_ip` |
| `src_port` | `src_port` |
| `dest_port` | `dst_port` |
| `proto` | `protocol` |
| `app_proto` | `app_protocol` |
| `flow.bytes_toserver + flow.bytes_toclient` | `bytes` |
| `flow.pkts_toserver + flow.pkts_toclient` | `packets` |
| Raw JSON object | `raw` |

Example command:

~~~bash
cd cpp-collector
./build/flow_collector --mode suricata --file ../samples/suricata_eve.json
~~~

## 4. NetFlow v5 UDP Collector

The NetFlow v5 collector listens on a UDP port and parses fixed-size NetFlow v5 binary records.

| NetFlow v5 Field | Normalized Field |
|---|---|
| `srcaddr` | `src_ip` |
| `dstaddr` | `dst_ip` |
| `srcport` | `src_port` |
| `dstport` | `dst_port` |
| `prot` | `protocol` |
| `dOctets` | `bytes` |
| `dPkts` | `packets` |
| `tcp_flags` | `tcp_flags` |
| Raw metadata | `raw` |

Example command:

~~~bash
cd cpp-collector
./build/flow_collector --mode netflow-v5 --port 2055
~~~

Sample packet generator:

~~~bash
python3 scripts/send_netflow_v5.py --count 2
~~~

## 5. NetFlow v9 MVP Collector

The NetFlow v9 collector listens on a UDP port and parses NetFlow v9 Template FlowSets and Data FlowSets.

This implementation is an MVP. It supports template-based parsing for common IPv4 flow fields.

| NetFlow v9 Field ID | Meaning | Normalized Field |
|---:|---|---|
| 1 | `IN_BYTES` | `bytes` |
| 2 | `IN_PKTS` | `packets` |
| 4 | `PROTOCOL` | `protocol` |
| 6 | `TCP_FLAGS` | `tcp_flags` |
| 7 | `L4_SRC_PORT` | `src_port` |
| 8 | `IPV4_SRC_ADDR` | `src_ip` |
| 11 | `L4_DST_PORT` | `dst_port` |
| 12 | `IPV4_DST_ADDR` | `dst_ip` |
| 21 | `LAST_SWITCHED` | `raw.last_switched` |
| 22 | `FIRST_SWITCHED` | `raw.first_switched` |

Example command:

~~~bash
cd cpp-collector
./build/flow_collector --mode netflow-v9 --port 9996
~~~

Sample packet generator:

~~~bash
python3 scripts/send_netflow_v9.py --port 9996 --count 2
~~~

## 6. Notes

- The collectors normalize records before sending them to the REST ingest API.
- The current sink is `HttpSink`, which posts records to `/api/v1/ingest/flows`.
- A Kafka/Redpanda sink can be added later by implementing the `ISink` interface.
- NetFlow v9 support is not a full implementation; it is an MVP for common IPv4 flow fields.
