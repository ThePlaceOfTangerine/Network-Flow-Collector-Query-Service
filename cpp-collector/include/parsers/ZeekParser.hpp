#pragma once

#include <QString>
#include "model/NormalizedFlow.hpp"

class ZeekParser {
public:
    bool parseLine(const QString& line, NormalizedFlow& output) const;

private:
    QString protocolToUpper(const QString& proto) const;
};
