#pragma once

#include <QByteArray>
#include <QVector>
#include <model/NormalizedFlow.hpp>

class NetflowV5Parser {
public:
    QVector<NormalizedFlow> parsePacket(const QByteArray& packet) const;

private:
    quint16 readU16(const char* data) const;
    quint32 readU32(const char* data) const;

    QString ipv4ToString(quint32 ip) const;
    QString protocolToString(quint8 protocol) const;
    QString tcpFlagsToString(quint8 flags) const;
};
