#include "Jobs.h"

#include "../config/Config.h"
#include "../db/ConnectionPool.h"
#include "../fmi/FmiClient.h"
#include "../fmi/WfsParser.h"
#include "../services/FetchLogService.h"
#include "../services/ForecastService.h"
#include "../services/ObservationService.h"
#include "../services/StationService.h"

#include <chrono>
#include <ctime>
#include <drogon/drogon.h>
#include <sstream>

namespace weather::scheduler {

// ─── Time helpers ──────────────────────────────────────────────────────────

static std::string fmtIso(std::chrono::system_clock::time_point tp) {
    time_t t = std::chrono::system_clock::to_time_t(tp);
    struct tm utc{};
    gmtime_r(&t, &utc);
    char buf[32];
    strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &utc);
    return buf;
}

// ─── fetchObservations ─────────────────────────────────────────────────────

void fetchObservations() {
    auto now       = std::chrono::system_clock::now();
    auto startTime = now - std::chrono::minutes(20);
    auto endTime   = now;

    std::string startIso = fmtIso(startTime);
    std::string endIso   = fmtIso(endTime);

    auto& pool   = getPool();
    auto& config = getConfig();

    std::string paramsJson =
        "{\"bbox\":\"" + config.fmi_bbox + "\","
        "\"start\":\"" + startIso + "\","
        "\"end\":\""   + endIso   + "\"}";

    long long logId = 0;
    try {
        logId = services::insertFetchLog(pool, "observations", paramsJson);
    } catch (const std::exception& e) {
        LOG_ERROR << "fetchObservations: insertFetchLog failed: " << e.what();
        logId = 0;
    }

    int count = 0;
    std::string errorMsg;

    try {
        fmi::FmiClient client;
        auto result = client.fetchObservations(startIso, endIso, config.fmi_bbox);
        if (!result.success) throw std::runtime_error(result.error);

        WfsParser parser;
        auto parsed = parser.parseObservations(result.xml);

        services::upsertStations(pool, parsed.stations);
        services::upsertObservations(pool, parsed.observations);
        count = static_cast<int>(parsed.observations.size());
        LOG_INFO << "fetchObservations: " << count << " records";
    } catch (const std::exception& e) {
        errorMsg = e.what();
        LOG_ERROR << "fetchObservations failed: " << errorMsg;
    }

    if (logId > 0) {
        try {
            std::optional<std::string> err;
            if (!errorMsg.empty()) err = errorMsg;
            services::updateFetchLog(pool, logId,
                errorMsg.empty() ? "success" : "error",
                count, err);
        } catch (...) {}
    }
}

// ─── fetchForecasts ────────────────────────────────────────────────────────

void fetchForecasts() {
    auto& pool   = getPool();
    auto& config = getConfig();

    auto now    = std::chrono::system_clock::now();
    std::string fetchedAt = fmtIso(now);

    for (const auto& place : config.default_places) {
        long long logId = 0;
        std::string paramsJson = "{\"place\":\"" + place + "\"}";
        try {
            logId = services::insertFetchLog(pool, "forecast_harmonie", paramsJson);
        } catch (...) {}

        int count = 0;
        std::string errorMsg;

        try {
            fmi::FmiClient client;
            auto result = client.fetchForecastHarmonie(place);
            if (!result.success) throw std::runtime_error(result.error);

            WfsParser parser;
            auto parsed = parser.parseForecasts(result.xml, place, fetchedAt);

            if (parsed.station) {
                std::vector<services::StationMeta> sm = {*parsed.station};
                services::upsertStations(pool, sm);
            }

            services::upsertForecasts(pool, parsed.forecasts);
            count = static_cast<int>(parsed.forecasts.size());
            LOG_INFO << "fetchForecasts[" << place << "]: " << count << " records";
        } catch (const std::exception& e) {
            errorMsg = e.what();
            LOG_ERROR << "fetchForecasts[" << place << "] failed: " << errorMsg;
        }

        if (logId > 0) {
            try {
                std::optional<std::string> err;
                if (!errorMsg.empty()) err = errorMsg;
                services::updateFetchLog(pool, logId,
                    errorMsg.empty() ? "success" : "error",
                    count, err);
            } catch (...) {}
        }
    }
}

} // namespace weather::scheduler
