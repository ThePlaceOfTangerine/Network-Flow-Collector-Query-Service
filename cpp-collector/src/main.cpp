#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "model/NormalizedFlow.hpp"
#include "sinks/HttpSink.hpp"

std::string now_iso() {
    return "2026-06-21T10:00:00";
}

NormalizedFlow make_fake_flow(int i) {
    NormalizedFlow f;

    f.event_id = "cpp-fake-" + std::to_string(i);
    f.source_type = "cpp_fake_collector";
    f.sensor_id = "cpp-collector-01";

    f.src_ip = "10.0.0." + std::to_string((i % 250) + 1);
    f.dst_ip = "8.8.8.8";

    f.src_port = 10000 + i;
    f.dst_port = 53;

    f.protocol = "UDP";
    f.bytes = 100 + i;
    f.packets = 2;

    f.start_time = now_iso();
    f.end_time = now_iso();
    f.duration = 0.1;

    f.app_protocol = "dns";
    f.tcp_flags = "";

    f.raw = {
        {"generator", "cpp_fake_collector"},
        {"index", i}
    };

    return f;
}

int main(int argc, char* argv[]) {
    std::string endpoint = "http://localhost:8000/api/v1/ingest/flows";
    int count = 10;

    if (argc >= 2) {
        count = std::stoi(argv[1]);
    }

    if (argc >= 3) {
        endpoint = argv[2];
    }

    std::cout << "FlowLog C++ Collector Skeleton\n";
    std::cout << "Sending " << count << " fake flows to " << endpoint << "\n";

    HttpSink sink(endpoint);

    std::vector<NormalizedFlow> batch;

    for (int i = 0; i < count; ++i) {
        batch.push_back(make_fake_flow(i));
    }

    bool ok = sink.send_batch(batch);

    if (!ok) {
        std::cerr << "Failed to publish fake flows\n";
        return 1;
    }

    std::cout << "Done\n";
    return 0;
}
