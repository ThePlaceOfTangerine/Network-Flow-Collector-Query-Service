#include "collectors/NetflowV9UdpCollector.hpp"

#include <QCoreApplication>
#include <QHostAddress>
#include <QNetworkDatagram>
#include <QDebug>

NetflowV9UdpCollector::NetflowV9UdpCollector(quint16 port, ISink& sink, QObject* parent)
    : QObject(parent), port_(port), sink_(sink) {}

void NetflowV9UdpCollector::run() {
    bool ok = socket_.bind(QHostAddress::AnyIPv4, port_);

    if (!ok) {
        qWarning() << "Failed to bind UDP port" << port_ << socket_.errorString();
        return;
    }

    qInfo() << "NetFlow v9 UDP collector listening on port" << port_;

    QObject::connect(
        &socket_,
        &QUdpSocket::readyRead,
        [this]() {
            handlePendingDatagrams();
        }
    );

    QCoreApplication::exec();
}

void NetflowV9UdpCollector::handlePendingDatagrams() {
    while (socket_.hasPendingDatagrams()) {
        QNetworkDatagram datagram = socket_.receiveDatagram();

        QByteArray payload = datagram.data();

        qInfo() << "Received NetFlow v9 UDP datagram from"
                << datagram.senderAddress().toString()
                << "port"
                << datagram.senderPort()
                << "bytes"
                << payload.size();

        QVector<NormalizedFlow> flows = parser_.parsePacket(payload);

        qInfo() << "Parsed NetFlow v9 records =" << flows.size();

        if (!flows.isEmpty()) {
            sink_.sendBatch(flows);
        }
    }
}
