#pragma once

// Curl stubs für Kompatibilität ohne externe curl-Abhängigkeit

typedef void CURL;
typedef enum {
    CURLE_OK = 0,
    CURLE_FAILED_INIT = 2,
    CURLE_UNSUPPORTED_PROTOCOL = 1
} CURLcode;

typedef enum {
    CURLOPT_URL = 10002,
    CURLOPT_WRITEFUNCTION = 20011,
    CURLOPT_WRITEDATA = 10001,
    CURLOPT_USERAGENT = 10018,
    CURLOPT_TIMEOUT = 13,
    CURLOPT_FOLLOWLOCATION = 52,
    CURLOPT_CONNECTTIMEOUT = 78
} CURLoption;

typedef enum {
    CURLINFO_RESPONSE_CODE = 0x200002,
    CURLINFO_SIZE_DOWNLOAD = 0x300008
} CURLINFO;

#define CURLVERSION_NOW 4

struct curl_version_info_data {
    int version;
    const char* version_num;
    const char* host;
    int features;
    const char* ssl_version;
    const char* libz_version;
};

// Stub-Implementierungen
inline CURL* curl_easy_init() { return nullptr; }
inline CURLcode curl_easy_setopt(CURL* curl, CURLoption option, ...) { return CURLE_FAILED_INIT; }
inline CURLcode curl_easy_perform(CURL* curl) { return CURLE_FAILED_INIT; }
inline void curl_easy_cleanup(CURL* curl) {}
inline CURLcode curl_easy_getinfo(CURL* curl, CURLINFO info, ...) { return CURLE_FAILED_INIT; }
inline const char* curl_easy_strerror(CURLcode error) { return "curl not available"; }
inline curl_version_info_data* curl_version_info(int type) {
    static curl_version_info_data info = {4, "7.0.0-stub", "stub", 0, "stub", "stub"};
    return &info;
}