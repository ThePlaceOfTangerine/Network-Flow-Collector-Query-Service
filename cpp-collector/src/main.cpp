#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDebug>

#include "sinks/HttpSink.hpp"
#include "collectors/FakeCollector.hpp"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    QCoreApplication::setApplicationName("flowlog-qt-collector");
    QCoreApplication::setApplicationVersion("0.1.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("Qt-based C++ collector for FlowLog system");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption countOption(
        QStringList() << "c" << "count",
        "Number of fake flows to generate.",
        "count",
        "10"
    );

    QCommandLineOption endpointOption(
        QStringList() << "e" << "endpoint",
        "REST ingest endpoint.",
        "endpoint",
        "http://localhost:8000/api/v1/ingest/flows"
    );

    parser.addOption(countOption);
    parser.addOption(endpointOption);

    parser.process(app);

    int count = parser.value(countOption).toInt();
    QString endpoint = parser.value(endpointOption);

    qInfo() << "FlowLog Qt C++ Collector";
    qInfo() << "count =" << count;
    qInfo() << "endpoint =" << endpoint;

    HttpSink sink(endpoint);
    FakeCollector collector(sink, count);

    collector.run();

    return 0;
}
