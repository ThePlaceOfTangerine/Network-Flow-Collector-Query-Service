#pragma once

#include <cstdint>
#include <string>
#include <nlohmann/json.hpp>

struct NormalizedFlow {
    std::string event_id;

    std::string source_type;
    std::string sensor_id;

    std::string src_ip;
    std::string dst_ip;

    uint16_t src_port = 0;
    uint16_t dst_port = 0;

    std::string protocol;

    uint64_t bytes = 0;
    uint64_t packets = 0;

    std::string start_time;
    std::string end_time;
    double duration = 0.0;

    std::string app_protocol;
    std::string tcp_flags;

    nlohmann::json raw;
};

inline nlohmann::json to_json(const NormalizedFlow& f) {
    return {
        {"event_id", f.event_id},
        {"source_type", f.source_type},
        {"sensor_id", f.sensor_id},
        {"src_ip", f.src_ip},
        {"dst_ip", f.dst_ip},
        {"src_port", f.src_port},
        {"dst_port", f.dst_port},
        {"protocol", f.protocol},
        {"bytes", f.bytes},
        {"packets", f.packets},
        {"start_time", f.start_time},
        {"end_time", f.end_time},
        {"duration", f.duration},
        {"app_protocol", f.app_protocol},
        {"tcp_flags", f.tcp_flags},
        {"raw", f.raw}
    };
}
