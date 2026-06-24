#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDebug>

#include "sinks/HttpSink.hpp"
#include "collectors/FakeCollector.hpp"
#include "collectors/FileCollector.hpp"
#include "collectors/NetflowUdpCollector.hpp"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    QCoreApplication::setApplicationName("flowlog-qt-collector");
    QCoreApplication::setApplicationVersion("0.3.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("Qt-based C++ collector for FlowLog system");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption modeOption(
        QStringList() << "m" << "mode",
        "Collector mode: fake, zeek, suricata, netflow-v5.",
        "mode",
        "fake"
    );

    QCommandLineOption countOption(
        QStringList() << "c" << "count",
        "Number of fake flows to generate.",
        "count",
        "10"
    );

    QCommandLineOption fileOption(
        QStringList() << "f" << "file",
        "Input log file path for zeek/suricata mode.",
        "file",
        ""
    );

    QCommandLineOption portOption(
        QStringList() << "p" << "port",
        "UDP port for netflow-v5 mode.",
        "port",
        "2055"
    );

    QCommandLineOption endpointOption(
        QStringList() << "e" << "endpoint",
        "REST ingest endpoint.",
        "endpoint",
        "http://localhost:8000/api/v1/ingest/flows"
    );

    parser.addOption(modeOption);
    parser.addOption(countOption);
    parser.addOption(fileOption);
    parser.addOption(portOption);
    parser.addOption(endpointOption);

    parser.process(app);

    QString mode = parser.value(modeOption);
    int count = parser.value(countOption).toInt();
    QString filePath = parser.value(fileOption);
    quint16 port = static_cast<quint16>(parser.value(portOption).toUShort());
    QString endpoint = parser.value(endpointOption);

    qInfo() << "FlowLog Qt C++ Collector";
    qInfo() << "mode =" << mode;
    qInfo() << "endpoint =" << endpoint;

    HttpSink sink(endpoint);

    if (mode == "fake") {
        FakeCollector collector(sink, count);
        collector.run();
        return 0;
    }

    if (mode == "zeek") {
        if (filePath.isEmpty()) {
            qWarning() << "Missing --file for zeek mode";
            return 1;
        }

        FileCollector collector(filePath, FileCollector::Format::Zeek, sink);
        collector.run();
        return 0;
    }

    if (mode == "suricata") {
        if (filePath.isEmpty()) {
            qWarning() << "Missing --file for suricata mode";
            return 1;
        }

        FileCollector collector(filePath, FileCollector::Format::Suricata, sink);
        collector.run();
        return 0;
    }

    if (mode == "netflow-v5") {
        NetflowUdpCollector collector(port, sink);
        collector.run();
        return 0;
    }

    qWarning() << "Unknown mode:" << mode;
    return 1;
}
