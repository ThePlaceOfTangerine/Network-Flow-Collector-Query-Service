#pragma once

#include <QObject>
#include <QUdpSocket>

#include "core/ICollector.hpp"
#include "core/ISink.hpp"
#include "parsers/NetflowV5Parser.hpp"

class NetflowUdpCollector : public QObject, public ICollector {
public:
    NetflowUdpCollector(quint16 port, ISink& sink, QObject* parent = nullptr);

    void run() override;

private:
    quint16 port_;
    ISink& sink_;
    QUdpSocket socket_;
    NetflowV5Parser parser_;

    void handlePendingDatagrams();
};
