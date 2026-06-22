#pragma once

#include <QString>
#include <QJsonObject>

class NormalizedFlow {
public:
    QString eventId;

    QString sourceType;
    QString sensorId;

    QString srcIp;
    QString dstIp;

    quint16 srcPort = 0;
    quint16 dstPort = 0;

    QString protocol;

    quint64 bytes = 0;
    quint64 packets = 0;

    QString startTime;
    QString endTime;

    double duration = 0.0;

    QString appProtocol;
    QString tcpFlags;

    QJsonObject raw;

    QJsonObject toJson() const;
};
