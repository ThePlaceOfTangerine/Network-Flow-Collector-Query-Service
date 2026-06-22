#include "collectors/FakeCollector.hpp"

#include <QJsonObject>

FakeCollector::FakeCollector(ISink& sink, int count)
    : sink_(sink), count_(count) {}

void FakeCollector::run() {
    QVector<NormalizedFlow> batch;

    for (int i = 0; i < count_; ++i) {
        batch.push_back(makeFlow(i));
    }

    sink_.sendBatch(batch);
}

NormalizedFlow FakeCollector::makeFlow(int index) const {
    NormalizedFlow f;

    f.eventId = "qt-fake-" + QString::number(index);
    f.sourceType = "cpp_qt_fake_collector";
    f.sensorId = "qt-collector-01";

    f.srcIp = "10.10.0." + QString::number((index % 250) + 1);
    f.dstIp = "8.8.8.8";

    f.srcPort = static_cast<quint16>(10000 + index);
    f.dstPort = 53;

    f.protocol = "UDP";

    f.bytes = 100 + index;
    f.packets = 2;

    f.startTime = "2026-06-21T10:00:00";
    f.endTime = "2026-06-21T10:00:01";

    f.duration = 1.0;

    f.appProtocol = "dns";
    f.tcpFlags = "";

    QJsonObject raw;
    raw["generator"] = "qt_fake_collector";
    raw["index"] = index;
    f.raw = raw;

    return f;
}
