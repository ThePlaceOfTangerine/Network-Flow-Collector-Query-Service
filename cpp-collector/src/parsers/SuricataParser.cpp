#include "parsers/SuricataParser.hpp"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

bool SuricataParser::parseLine(const QString& line, NormalizedFlow& output) const {
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(line.toUtf8(), &error);

    if (error.error != QJsonParseError::NoError || !doc.isObject()) {
        return false;
    }

    QJsonObject j = doc.object();

    QString eventType = j.value("event_type").toString();

    if (eventType != "flow") {
        return false;
    }

    QString srcIp = j.value("src_ip").toString();
    QString dstIp = j.value("dest_ip").toString();

    if (srcIp.isEmpty() || dstIp.isEmpty()) {
        return false;
    }

    QJsonObject flow = j.value("flow").toObject();

    quint64 bytesToServer = static_cast<quint64>(flow.value("bytes_toserver").toDouble(0));
    quint64 bytesToClient = static_cast<quint64>(flow.value("bytes_toclient").toDouble(0));

    quint64 pktsToServer = static_cast<quint64>(flow.value("pkts_toserver").toDouble(0));
    quint64 pktsToClient = static_cast<quint64>(flow.value("pkts_toclient").toDouble(0));

    QString flowId = QString::number(static_cast<quint64>(j.value("flow_id").toDouble(0)));

    output.eventId = "suricata-" + flowId;
    output.sourceType = "suricata";
    output.sensorId = "suricata-file";

    output.srcIp = srcIp;
    output.dstIp = dstIp;

    output.srcPort = static_cast<quint16>(j.value("src_port").toInt(0));
    output.dstPort = static_cast<quint16>(j.value("dest_port").toInt(0));

    output.protocol = j.value("proto").toString().toUpper();

    output.bytes = bytesToServer + bytesToClient;
    output.packets = pktsToServer + pktsToClient;

    output.startTime = "2026-06-21T10:00:00";
    output.endTime = "2026-06-21T10:00:01";

    output.duration = 0.0;

    output.appProtocol = j.value("app_proto").toString();
    output.tcpFlags = "";

    output.raw = j;

    return true;
}
