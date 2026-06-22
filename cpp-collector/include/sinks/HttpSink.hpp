#pragma once

#include <curl/curl.h>
#include <iostream>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

#include "core/ISink.hpp"
#include "model/NormalizedFlow.hpp"

class HttpSink : public ISink {
private:
    std::string endpoint_;

public:
    explicit HttpSink(std::string endpoint)
        : endpoint_(std::move(endpoint)) {
        curl_global_init(CURL_GLOBAL_DEFAULT);
    }

    ~HttpSink() override {
        curl_global_cleanup();
    }

    bool send(const NormalizedFlow& flow) override {
        std::vector<NormalizedFlow> batch = {flow};
        return send_batch(batch);
    }

    bool send_batch(const std::vector<NormalizedFlow>& flows) override {
        CURL* curl = curl_easy_init();

        if (!curl) {
            std::cerr << "Failed to initialize CURL\n";
            return false;
        }

        nlohmann::json payload = nlohmann::json::array();

        for (const auto& flow : flows) {
            payload.push_back(to_json(flow));
        }

        std::string body = payload.dump();

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, endpoint_.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, body.size());

        CURLcode res = curl_easy_perform(curl);

        long response_code = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            std::cerr << "HTTP sink error: " << curl_easy_strerror(res) << "\n";
            return false;
        }

        if (response_code < 200 || response_code >= 300) {
            std::cerr << "HTTP sink returned status: " << response_code << "\n";
            std::cerr << "Payload: " << body << "\n";
            return false;
        }

        std::cout << "Published batch size=" << flows.size()
                  << " status=" << response_code << "\n";

        return true;
    }
};
