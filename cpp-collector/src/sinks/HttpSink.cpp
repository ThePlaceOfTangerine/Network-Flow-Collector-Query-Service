#include "sinks/HttpSink.hpp"

#include <QEventLoop>
#include <QJsonArray>
#include <QJsonDocument>
#include <QNetworkRequest>
#include <QDebug>

HttpSink::HttpSink(const QString& endpoint, QObject* parent)
    : QObject(parent), endpoint_(endpoint) {}

bool HttpSink::send(const NormalizedFlow& flow) {
    QVector<NormalizedFlow> batch;
    batch.push_back(flow);
    return sendBatch(batch);
}

bool HttpSink::sendBatch(const QVector<NormalizedFlow>& flows) {
    return postJsonArray(flows);
}

bool HttpSink::postJsonArray(const QVector<NormalizedFlow>& flows) {
    QJsonArray arr;

    for (const auto& flow : flows) {
        arr.append(flow.toJson());
    }

    QJsonDocument doc(arr);
    QByteArray body = doc.toJson(QJsonDocument::Compact);

    QNetworkRequest request;
    request.setUrl(QUrl(endpoint_));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply* reply = manager_.post(request, body);

    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "HTTP sink error:" << reply->errorString();
        reply->deleteLater();
        return false;
    }

    QByteArray responseBody = reply->readAll();

    qInfo() << "Published batch size =" << flows.size()
            << "status =" << statusCode
            << "response =" << responseBody;

    reply->deleteLater();

    return statusCode >= 200 && statusCode < 300;
}
