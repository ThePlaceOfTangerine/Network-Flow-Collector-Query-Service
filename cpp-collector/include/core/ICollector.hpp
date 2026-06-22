#pragma once

class ICollector {
public:
    virtual ~ICollector() = default;
    virtual void run() = 0;
};
