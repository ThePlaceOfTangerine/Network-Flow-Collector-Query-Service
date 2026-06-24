#include "parsers/NetflowV5Parser.hpp"

#include <QJsonObject>
#include <QString>

quint16 NetflowV5Parser::readU16(const char* data) const {
    const unsigned char* p = reinterpret_cast<const unsigned char*>(data);
    return static_cast<quint16>((p[0] << 8) | p[1]);
}

quint32 NetflowV5Parser::readU32(const char* data) const {
    const unsigned char* p = reinterpret_cast<const unsigned char*>(data);
    return (static_cast<quint32>(p[0]) << 24)
         | (static_cast<quint32>(p[1]) << 16)
         | (static_cast<quint32>(p[2]) << 8)
         | static_cast<quint32>(p[3]);
}

QString NetflowV5Parser::ipv4ToString(quint32 ip) const {
    return QString("%1.%2.%3.%4")
        .arg((ip >> 24) & 0xFF)
        .arg((ip >> 16) & 0xFF)
        .arg((ip >> 8) & 0xFF)
        .arg(ip & 0xFF);
}

QString NetflowV5Parser::protocolToString(quint8 protocol) const {
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

QString NetflowV5Parser::tcpFlagsToString(quint8 flags) const {
    QStringList names;

    if (flags & 0x01) names << "FIN";
    if (flags & 0x02) names << "SYN";
    if (flags & 0x04) names << "RST";
    if (flags & 0x08) names << "PSH";
    if (flags & 0x10) names << "ACK";
    if (flags & 0x20) names << "URG";

    return names.join("|");
}

QVector<NormalizedFlow> NetflowV5Parser::parsePacket(const QByteArray& packet) const {
    QVector<NormalizedFlow> flows;

    constexpr int HEADER_SIZE = 24;
    constexpr int RECORD_SIZE = 48;

    if (packet.size() < HEADER_SIZE) {
        return flows;
    }

    const char* data = packet.constData();

    quint16 version = readU16(data);
    quint16 count = readU16(data + 2);

    if (version != 5) {
        return flows;
    }

    int availableRecords = (packet.size() - HEADER_SIZE) / RECORD_SIZE;
    int recordCount = qMin(static_cast<int>(count), availableRecords);

    quint32 unixSecs = readU32(data + 8);
    Q_UNUSED(unixSecs);

    for (int i = 0; i < recordCount; ++i) {
        const char* r = data + HEADER_SIZE + i * RECORD_SIZE;

        quint32 srcaddr = readU32(r + 0);
        quint32 dstaddr = readU32(r + 4);

        quint32 dPkts = readU32(r + 16);
        quint32 dOctets = readU32(r + 20);

        quint32 first = readU32(r + 24);
        quint32 last = readU32(r + 28);

        quint16 srcport = readU16(r + 32);
        quint16 dstport = readU16(r + 34);

        quint8 tcpFlags = static_cast<quint8>(r[37]);
        quint8 protocol = static_cast<quint8>(r[38]);

        NormalizedFlow f;

        f.eventId = "netflow-v5-" + QString::number(unixSecs) + "-" + QString::number(i);
        f.sourceType = "netflow_v5";
        f.sensorId = "udp-collector-2055";

        f.srcIp = ipv4ToString(srcaddr);
        f.dstIp = ipv4ToString(dstaddr);

        f.srcPort = srcport;
        f.dstPort = dstport;

        f.protocol = protocolToString(protocol);

        f.bytes = dOctets;
        f.packets = dPkts;

        f.startTime = "2026-06-21T10:00:00";
        f.endTime = "2026-06-21T10:00:01";
        f.duration = static_cast<double>(last > first ? last - first : 0) / 1000.0;

        f.appProtocol = "";
        f.tcpFlags = tcpFlagsToString(tcpFlags);

        QJsonObject raw;
        raw["version"] = static_cast<int>(version);
        raw["record_index"] = i;
        raw["first"] = static_cast<double>(first);
        raw["last"] = static_cast<double>(last);
        raw["tcp_flags_raw"] = static_cast<int>(tcpFlags);
        raw["protocol_raw"] = static_cast<int>(protocol);

        f.raw = raw;

        flows.push_back(f);
    }

    return flows;
}
