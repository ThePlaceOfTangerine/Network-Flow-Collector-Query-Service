#include "collectors/FileCollector.hpp"

#include <QFile>
#include <QTextStream>
#include <QVector>
#include <QDebug>

#include "parsers/ZeekParser.hpp"
#include "parsers/SuricataParser.hpp"
#include "model/NormalizedFlow.hpp"

FileCollector::FileCollector(const QString& filePath, Format format, ISink& sink)
    : filePath_(filePath), format_(format), sink_(sink) {}

void FileCollector::run() {
    QFile file(filePath_);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Cannot open file:" << filePath_;
        return;
    }

    QTextStream in(&file);

    QVector<NormalizedFlow> batch;
    int parsed = 0;
    int failed = 0;

    ZeekParser zeekParser;
    SuricataParser suricataParser;

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();

        if (line.isEmpty()) {
            continue;
        }

        NormalizedFlow flow;
        bool ok = false;

        if (format_ == Format::Zeek) {
            ok = zeekParser.parseLine(line, flow);
        } else if (format_ == Format::Suricata) {
            ok = suricataParser.parseLine(line, flow);
        }

        if (ok) {
            batch.push_back(flow);
            parsed++;
        } else {
            failed++;
        }
    }

    qInfo() << "FileCollector parsed =" << parsed << "failed =" << failed;

    if (!batch.isEmpty()) {
        sink_.sendBatch(batch);
    }
}
