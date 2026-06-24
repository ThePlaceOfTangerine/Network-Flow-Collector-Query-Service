#pragma once

#include <QString>
#include "model/NormalizedFlow.hpp"

class SuricataParser {
public:
    bool parseLine(const QString& line, NormalizedFlow& output) const;
};
