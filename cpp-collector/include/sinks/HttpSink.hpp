#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>
#include <QVector>

#include "core/ISink.hpp"
#include "model/NormalizedFlow.hpp"

class HttpSink : public QObject, public ISink {

public:
    explicit HttpSink(const QString& endpoint, QObject* parent = nullptr);

    bool send(const NormalizedFlow& flow) override;
    bool sendBatch(const QVector<NormalizedFlow>& flows) override;

private:
    QString endpoint_;
    QNetworkAccessManager manager_;

    bool postJsonArray(const QVector<NormalizedFlow>& flows);
};
