# Collectors

## Zeek File Collector

Reads Zeek `conn.log` JSON lines and maps each connection record into the normalized flow schema.

### Field Mapping

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
| raw JSON object | `raw` |

## Suricata File Collector

Reads Suricata `eve.json` records where `event_type = flow`.

### Field Mapping

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
| raw JSON object | `raw` |
