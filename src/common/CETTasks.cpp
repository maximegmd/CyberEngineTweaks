#include <stdafx.h>
#include <curl/curl.h>

#include "CETTasks.h"

CETTasks::CETTasks(Options& aOptions)
    : m_running(true)
{
    curl_global_init(CURL_GLOBAL_ALL);

    if (aOptions.Telemetry)
        m_telemetryThread = std::thread(&CETTasks::RunTelemetry, this);
}

CETTasks::~CETTasks()
{
    m_running.store(false);

    m_telemetryThread.join();

    curl_global_cleanup();
}

auto GenerateRandomString(std::size_t len) -> std::string
{
    auto randchar = []() -> char {
        const char charset[] = "0123456789"
                               "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                               "abcdefghijklmnopqrstuvwxyz";
        const size_t max_index = (sizeof(charset) - 1);
        return charset[rand() % max_index];
    };
    std::string str(len, 0);
    std::generate_n(str.begin(), len, randchar);
    return str;
}

void CETTasks::RunTelemetry()
{
    const auto cName = "name=" + GenerateRandomString(64);

    while(m_running.load())
    {
        const auto cpCurl = curl_easy_init();
        if (cpCurl)
        {
            curl_easy_setopt(cpCurl, CURLOPT_URL, "https://cet.tiltedphoques.com/announce");
            curl_easy_setopt(cpCurl, CURLOPT_POSTFIELDS, cName.c_str());

            const auto res = curl_easy_perform(cpCurl);

            if (res != CURLE_OK)
                spdlog::error("curl_easy_perform() failed: {}", curl_easy_strerror(res));

            curl_easy_cleanup(cpCurl);
        }

        std::this_thread::sleep_for(std::chrono::seconds(60));
    }
}
