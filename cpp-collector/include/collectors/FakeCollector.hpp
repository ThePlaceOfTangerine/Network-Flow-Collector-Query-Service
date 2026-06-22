#pragma once

#include <QString>
#include <QVector>

#include "core/ICollector.hpp"
#include "core/ISink.hpp"
#include "model/NormalizedFlow.hpp"

class FakeCollector : public ICollector {
public:
    FakeCollector(ISink& sink, int count);

    void run() override;

private:
    ISink& sink_;
    int count_;

    NormalizedFlow makeFlow(int index) const;
};
