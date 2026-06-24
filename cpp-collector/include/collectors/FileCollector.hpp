#pragma once

#include <QString>

#include "core/ICollector.hpp"
#include "core/ISink.hpp"

class FileCollector : public ICollector {
public:
    enum class Format {
        Zeek,
        Suricata
    };

    FileCollector(const QString& filePath, Format format, ISink& sink);

    void run() override;

private:
    QString filePath_;
    Format format_;
    ISink& sink_;
};
