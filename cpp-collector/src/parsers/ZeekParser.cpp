#include "parsers/ZeekParser.hpp"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

QString ZeekParser::protocolToUpper(const QString& proto) const {
    return proto.toUpper();
}

bool ZeekParser::parseLine(const QString& line, NormalizedFlow& output) const {
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(line.toUtf8(), &error);

    if (error.error != QJsonParseError::NoError || !doc.isObject()) {
        return false;
    }

    QJsonObject j = doc.object();

    QString srcIp = j.value("id.orig_h").toString();
    QString dstIp = j.value("id.resp_h").toString();

    if (srcIp.isEmpty() || dstIp.isEmpty()) {
        return false;
    }

    int srcPort = j.value("id.orig_p").toInt(0);
    int dstPort = j.value("id.resp_p").toInt(0);

    double origBytes = j.value("orig_bytes").toDouble(0);
    double respBytes = j.value("resp_bytes").toDouble(0);

    double origPkts = j.value("orig_pkts").toDouble(0);
    double respPkts = j.value("resp_pkts").toDouble(0);

    QString uid = j.value("uid").toString();

    output.eventId = uid.isEmpty()
        ? "zeek-" + QString::number(qHash(line))
        : "zeek-" + uid;

    output.sourceType = "zeek";
    output.sensorId = "zeek-file";

    output.srcIp = srcIp;
    output.dstIp = dstIp;

    output.srcPort = static_cast<quint16>(srcPort);
    output.dstPort = static_cast<quint16>(dstPort);

    output.protocol = protocolToUpper(j.value("proto").toString());

    output.bytes = static_cast<quint64>(origBytes + respBytes);
    output.packets = static_cast<quint64>(origPkts + respPkts);

    output.startTime = "2026-06-21T10:00:00";
    output.endTime = "2026-06-21T10:00:01";

    output.duration = j.value("duration").toDouble(0.0);

    output.appProtocol = j.value("service").toString();
    output.tcpFlags = "";

    output.raw = j;

    return true;
}
