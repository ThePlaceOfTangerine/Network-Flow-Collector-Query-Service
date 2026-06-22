#include "model/NormalizedFlow.hpp"

QJsonObject NormalizedFlow::toJson() const {
    QJsonObject obj;

    obj["event_id"] = eventId;
    obj["source_type"] = sourceType;
    obj["sensor_id"] = sensorId;

    obj["src_ip"] = srcIp;
    obj["dst_ip"] = dstIp;

    obj["src_port"] = static_cast<int>(srcPort);
    obj["dst_port"] = static_cast<int>(dstPort);

    obj["protocol"] = protocol;

    obj["bytes"] = static_cast<double>(bytes);
    obj["packets"] = static_cast<double>(packets);

    obj["start_time"] = startTime;
    obj["end_time"] = endTime;

    obj["duration"] = duration;

    obj["app_protocol"] = appProtocol;
    obj["tcp_flags"] = tcpFlags;

    obj["raw"] = raw;

    return obj;
}
