#include "HealthController.h"

#include "../db/ConnectionPool.h"
#include <drogon/HttpResponse.h>
#include <json/json.h>
#include <pqxx/pqxx>
#include <chrono>
#include <ctime>

namespace weather {

void HealthController::health(const drogon::HttpRequestPtr& req,
                               std::function<void(const drogon::HttpResponsePtr&)>&& callback)
{
    auto now = std::chrono::system_clock::now();
    time_t t = std::chrono::system_clock::to_time_t(now);
    struct tm utc;
    gmtime_r(&t, &utc);
    char buf[32];
    strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &utc);
    std::string serverTime = buf;

    Json::Value j;
    j["server_time"]     = serverTime;
    j["last_observation"] = Json::Value();
    j["last_fetch"]       = Json::Value();

    try {
        auto conn = getPool().acquire();
        pqxx::work txn(conn.get());

        auto r1 = txn.exec(
            "SELECT to_char(MAX(time) AT TIME ZONE 'UTC', "
            "       'YYYY-MM-DD\"T\"HH24:MI:SS\"Z\"') AS t FROM observations"
        );
        if (!r1.empty() && !r1[0][0].is_null())
            j["last_observation"] = r1[0][0].as<std::string>();

        auto r2 = txn.exec(
            "SELECT to_char(MAX(finished_at) AT TIME ZONE 'UTC', "
            "       'YYYY-MM-DD\"T\"HH24:MI:SS\"Z\"') AS t "
            "FROM fetch_log WHERE status='success'"
        );
        if (!r2.empty() && !r2[0][0].is_null())
            j["last_fetch"] = r2[0][0].as<std::string>();

        j["status"]       = "ok";
        j["db_connected"] = true;
    } catch (...) {
        j["status"]       = "degraded";
        j["db_connected"] = false;
    }

    callback(drogon::HttpResponse::newHttpJsonResponse(j));
}

} // namespace weather
