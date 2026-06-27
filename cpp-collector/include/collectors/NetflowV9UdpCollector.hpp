#pragma once

#include <QObject>
#include <QUdpSocket>

#include "core/ICollector.hpp"
#include "core/ISink.hpp"
#include "parsers/NetflowV9Parser.hpp"

class NetflowV9UdpCollector : public QObject, public ICollector {
public:
    NetflowV9UdpCollector(quint16 port, ISink& sink, QObject* parent = nullptr);

    void run() override;

private:
    quint16 port_;
    ISink& sink_;
    QUdpSocket socket_;
    NetflowV9Parser parser_;

    void handlePendingDatagrams();
};
