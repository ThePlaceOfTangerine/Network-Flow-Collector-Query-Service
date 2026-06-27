#include "parsers/NetflowV9Parser.hpp"

#include <QJsonObject>
#include <QStringList>
#include <QDebug>

quint8 NetflowV9Parser::readU8(const char* data) const {
    const unsigned char* p = reinterpret_cast<const unsigned char*>(data);
    return static_cast<quint8>(p[0]);
}

quint16 NetflowV9Parser::readU16(const char* data) const {
    const unsigned char* p = reinterpret_cast<const unsigned char*>(data);
    return static_cast<quint16>((p[0] << 8) | p[1]);
}

quint32 NetflowV9Parser::readU32(const char* data) const {
    const unsigned char* p = reinterpret_cast<const unsigned char*>(data);
    return (static_cast<quint32>(p[0]) << 24)
         | (static_cast<quint32>(p[1]) << 16)
         | (static_cast<quint32>(p[2]) << 8)
         | static_cast<quint32>(p[3]);
}

quint64 NetflowV9Parser::readUInt(const char* data, int length) const {
    const unsigned char* p = reinterpret_cast<const unsigned char*>(data);
    quint64 value = 0;

    for (int i = 0; i < length; ++i) {
        value = (value << 8) | p[i];
    }

    return value;
}

QString NetflowV9Parser::ipv4ToString(quint32 ip) const {
    return QString("%1.%2.%3.%4")
        .arg((ip >> 24) & 0xFF)
        .arg((ip >> 16) & 0xFF)
        .arg((ip >> 8) & 0xFF)
        .arg(ip & 0xFF);
}

QString NetflowV9Parser::protocolToString(quint8 protocol) const {
    if (protocol == 6) {
        return "TCP";
    }

    if (protocol == 17) {
        return "UDP";
    }

    if (protocol == 1) {
        return "ICMP";
    }

    return QString::number(protocol);
}

QString NetflowV9Parser::tcpFlagsToString(quint8 flags) const {
    QStringList names;

    if (flags & 0x01) names << "FIN";
    if (flags & 0x02) names << "SYN";
    if (flags & 0x04) names << "RST";
    if (flags & 0x08) names << "PSH";
    if (flags & 0x10) names << "ACK";
    if (flags & 0x20) names << "URG";

    return names.join("|");
}

void NetflowV9Parser::parseTemplateFlowSet(const char* data, int length) {
    int offset = 0;

    while (offset + 4 <= length) {
        quint16 templateId = readU16(data + offset);
        quint16 fieldCount = readU16(data + offset + 2);
        offset += 4;

        QVector<FieldSpec> fields;
        bool valid = true;

        for (int i = 0; i < fieldCount; ++i) {
            if (offset + 4 > length) {
                valid = false;
                break;
            }

            FieldSpec field;
            field.type = readU16(data + offset);
            field.length = readU16(data + offset + 2);

            fields.push_back(field);
            offset += 4;
        }

        if (valid && templateId >= 256 && !fields.isEmpty()) {
            templates_[templateId] = fields;
            qInfo() << "Stored NetFlow v9 template"
                    << templateId
                    << "fields"
                    << fields.size();
        } else {
            break;
        }
    }
}

QVector<NormalizedFlow> NetflowV9Parser::parseDataFlowSet(
    quint16 templateId,
    const char* data,
    int length,
    quint32 unixSecs,
    quint32 sourceId
) {
    QVector<NormalizedFlow> flows;

    if (!templates_.contains(templateId)) {
        qWarning() << "Missing NetFlow v9 template" << templateId;
        return flows;
    }

    QVector<FieldSpec> fields = templates_.value(templateId);

    int recordLength = 0;
    for (const auto& field : fields) {
        recordLength += field.length;
    }

    if (recordLength <= 0) {
        return flows;
    }

    int offset = 0;

    while (offset + recordLength <= length) {
        const char* record = data + offset;

        NormalizedFlow f;

        f.sourceType = "netflow_v9";
        f.sensorId = "udp-collector-v9";
        f.eventId = "netflow-v9-" + QString::number(unixSecs)
                  + "-" + QString::number(sourceId)
                  + "-" + QString::number(flows.size());

        f.startTime = "2026-06-21T10:00:00";
        f.endTime = "2026-06-21T10:00:01";

        quint32 firstSwitched = 0;
        quint32 lastSwitched = 0;

        QJsonObject raw;
        raw["template_id"] = static_cast<int>(templateId);
        raw["source_id"] = static_cast<double>(sourceId);

        int fieldOffset = 0;

        for (const auto& field : fields) {
            const char* valuePtr = record + fieldOffset;

            switch (field.type) {
                case 1: // IN_BYTES
                    f.bytes = readUInt(valuePtr, field.length);
                    raw["in_bytes"] = static_cast<double>(f.bytes);
                    break;

                case 2: // IN_PKTS
                    f.packets = readUInt(valuePtr, field.length);
                    raw["in_pkts"] = static_cast<double>(f.packets);
                    break;

                case 4: // PROTOCOL
                    if (field.length >= 1) {
                        quint8 p = readU8(valuePtr);
                        f.protocol = protocolToString(p);
                        raw["protocol_raw"] = static_cast<int>(p);
                    }
                    break;

                case 6: // TCP_FLAGS
                    if (field.length >= 1) {
                        quint8 flags = readU8(valuePtr);
                        f.tcpFlags = tcpFlagsToString(flags);
                        raw["tcp_flags_raw"] = static_cast<int>(flags);
                    }
                    break;

                case 7: // L4_SRC_PORT
                    if (field.length == 2) {
                        f.srcPort = readU16(valuePtr);
                    }
                    break;

                case 8: // IPV4_SRC_ADDR
                    if (field.length == 4) {
                        f.srcIp = ipv4ToString(readU32(valuePtr));
                    }
                    break;

                case 11: // L4_DST_PORT
                    if (field.length == 2) {
                        f.dstPort = readU16(valuePtr);
                    }
                    break;

                case 12: // IPV4_DST_ADDR
                    if (field.length == 4) {
                        f.dstIp = ipv4ToString(readU32(valuePtr));
                    }
                    break;

                case 21: // LAST_SWITCHED
                    if (field.length == 4) {
                        lastSwitched = readU32(valuePtr);
                        raw["last_switched"] = static_cast<double>(lastSwitched);
                    }
                    break;

                case 22: // FIRST_SWITCHED
                    if (field.length == 4) {
                        firstSwitched = readU32(valuePtr);
                        raw["first_switched"] = static_cast<double>(firstSwitched);
                    }
                    break;

                default:
                    break;
            }

            fieldOffset += field.length;
        }

        if (lastSwitched > firstSwitched) {
            f.duration = static_cast<double>(lastSwitched - firstSwitched) / 1000.0;
        } else {
            f.duration = 0.0;
        }

        f.appProtocol = "";
        f.raw = raw;

        if (!f.srcIp.isEmpty() && !f.dstIp.isEmpty()) {
            flows.push_back(f);
        }

        offset += recordLength;
    }

    return flows;
}

QVector<NormalizedFlow> NetflowV9Parser::parsePacket(const QByteArray& packet) {
    QVector<NormalizedFlow> result;

    constexpr int HEADER_SIZE = 20;

    if (packet.size() < HEADER_SIZE) {
        return result;
    }

    const char* data = packet.constData();

    quint16 version = readU16(data);
    quint16 count = readU16(data + 2);
    Q_UNUSED(count);

    if (version != 9) {
        return result;
    }

    quint32 unixSecs = readU32(data + 8);
    quint32 sourceId = readU32(data + 16);

    int offset = HEADER_SIZE;

    while (offset + 4 <= packet.size()) {
        quint16 flowSetId = readU16(data + offset);
        quint16 flowSetLength = readU16(data + offset + 2);

        if (flowSetLength < 4 || offset + flowSetLength > packet.size()) {
            break;
        }

        const char* flowSetData = data + offset + 4;
        int flowSetPayloadLength = flowSetLength - 4;

        if (flowSetId == 0) {
            parseTemplateFlowSet(flowSetData, flowSetPayloadLength);
        } else if (flowSetId >= 256) {
            QVector<NormalizedFlow> flows = parseDataFlowSet(
                flowSetId,
                flowSetData,
                flowSetPayloadLength,
                unixSecs,
                sourceId
            );

            result += flows;
        }

        offset += flowSetLength;
    }

    return result;
}
