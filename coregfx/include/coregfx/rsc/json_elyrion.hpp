#pragma once

#include <string>
#include <fstream>
#include <streambuf>
#include <iostream>
#include "../../../adapters/curl_stubs.h"
#include <google/protobuf/util/json_util.h>
#include "nyx1.grpc.pb.h"

namespace jsonelyrion::ElyrionLoader {

    namespace {
        // --- Helper for libcurl ---
        inline size_t writeToString(void* contents, size_t size, size_t nmemb, void* userp) {
            size_t totalSize = size * nmemb;
            static_cast<std::string*>(userp)->append(static_cast<char*>(contents), totalSize);
            return totalSize;
        }

        inline bool loadContentFromFile(std::string_view path, std::string& out) {
            std::ifstream file(std::string(path), std::ios::in | std::ios::binary);
            if (!file.is_open()) return false;
            file.seekg(0, std::ios::end);
            out.resize(file.tellg());
            file.seekg(0, std::ios::beg);
            file.read(&out[0], out.size());
            return true;
        }

        inline bool loadContentFromUri(std::string_view uri, std::string& out) {
            CURL* curl = curl_easy_init();
            if (!curl) return false;

            curl_easy_setopt(curl, CURLOPT_URL, std::string(uri).c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeToString);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &out);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
            curl_easy_setopt(curl, CURLOPT_USERAGENT, "jsonelyrion/1.0");

            CURLcode res = curl_easy_perform(curl);
            curl_easy_cleanup(curl);

            return res == CURLE_OK;
        }

        inline bool parseJsonToModel(const std::string& json, nyx::Elyrion& model) {
            auto status = google::protobuf::util::JsonStringToMessage(json, &model);
            if (!status.ok()) {
                std::cerr << "Failed to parse JSON: " << status.ToString() << "\n";
                return false;
            }
            return true;
        }
    }

    // --- Public API ---

    inline bool loadJsonFromFile(std::string_view path, nyx::Elyrion& model) {
        std::string json;
        if (!loadContentFromFile(path, json)) {
            std::cerr << "Failed to read JSON file: " << path << "\n";
            return false;
        }
        return parseJsonToModel(json, model);
    }

    inline bool loadJsonFromUri(std::string_view uri, nyx::Elyrion& model) {
        std::string json;
        if (!loadContentFromUri(uri, json)) {
            std::cerr << "Failed to fetch JSON from URI: " << uri << "\n";
            return false;
        }
        return parseJsonToModel(json, model);
    }
}
