#pragma once

#include <QByteArray>
#include <QHash>
#include <QVector>

#include "model/NormalizedFlow.hpp"

class NetflowV9Parser {
public:
    QVector<NormalizedFlow> parsePacket(const QByteArray& packet);

private:
    struct FieldSpec {
        quint16 type = 0;
        quint16 length = 0;
    };

    QHash<quint16, QVector<FieldSpec>> templates_;

    quint8 readU8(const char* data) const;
    quint16 readU16(const char* data) const;
    quint32 readU32(const char* data) const;
    quint64 readUInt(const char* data, int length) const;

    QString ipv4ToString(quint32 ip) const;
    QString protocolToString(quint8 protocol) const;
    QString tcpFlagsToString(quint8 flags) const;

    void parseTemplateFlowSet(const char* data, int length);

    QVector<NormalizedFlow> parseDataFlowSet(
        quint16 templateId,
        const char* data,
        int length,
        quint32 unixSecs,
        quint32 sourceId
    );
};
