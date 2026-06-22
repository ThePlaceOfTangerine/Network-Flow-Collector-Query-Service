#pragma once

#include <string>
#include <vector>
#include "model/NormalizedFlow.hpp"

class IParser {
public:
    virtual ~IParser() = default;

    virtual bool parse_line(const std::string& input, NormalizedFlow& output) {
        return false;
    }

    virtual std::vector<NormalizedFlow> parse_packet(const uint8_t* data, size_t length) {
        return {};
    }
};
