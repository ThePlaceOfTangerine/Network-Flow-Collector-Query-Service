#pragma once

#include <QVector>
#include "model/NormalizedFlow.hpp"

class ISink {
public:
    virtual ~ISink() = default;

    virtual bool send(const NormalizedFlow& flow) = 0;

    virtual bool sendBatch(const QVector<NormalizedFlow>& flows) {
        for (const auto& flow : flows) {
            if (!send(flow)) {
                return false;
            }
        }
        return true;
    }
};
