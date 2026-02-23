#include "ObservationService.h"

#include <pqxx/pqxx>
#include <chrono>
#include <ctime>
#include <sstream>

namespace weather::services {

// ─── Helpers ───────────────────────────────────────────────────────────────

static std::string nowIso() {
    auto now = std::chrono::system_clock::now();
    time_t t  = std::chrono::system_clock::to_time_t(now);
    struct tm utc;
    gmtime_r(&t, &utc);
    char buf[32];
    strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &utc);
    return buf;
}

static std::string hoursAgoIso(int hours) {
    auto tp = std::chrono::system_clock::now() - std::chrono::hours(hours);
    time_t t = std::chrono::system_clock::to_time_t(tp);
    struct tm utc;
    gmtime_r(&t, &utc);
    char buf[32];
    strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &utc);
    return buf;
}

// ─── Upsert ────────────────────────────────────────────────────────────────

void upsertObservations(ConnectionPool& pool,
                        const std::vector<Observation>& observations)
{
    if (observations.empty()) return;
    auto conn = pool.acquire();
    pqxx::work txn(conn.get());

    for (const auto& o : observations) {
        txn.exec_params(
            "INSERT INTO observations "
            "(time, fmisid, temperature, dew_point, humidity, wind_speed, wind_gust, "
            " wind_direction, precipitation_1h, precip_intensity, snow_depth, "
            " pressure, visibility, cloud_cover, weather_code) "
            "VALUES ($1,$2,$3,$4,$5,$6,$7,$8,$9,$10,$11,$12,$13,$14,$15) "
            "ON CONFLICT (time, fmisid) DO NOTHING",
            o.time, o.fmisid,
            o.temperature, o.dew_point, o.humidity,
            o.wind_speed, o.wind_gust, o.wind_direction,
            o.precipitation_1h, o.precip_intensity, o.snow_depth,
            o.pressure, o.visibility, o.cloud_cover, o.weather_code
        );
    }
    txn.commit();
}

// ─── Time-series ───────────────────────────────────────────────────────────

Json::Value getObservations(ConnectionPool& pool,
                            int fmisid,
                            const std::string& startIn,
                            const std::string& endIn,
                            const std::string& resolutionIn)
{
    std::string start      = startIn.empty()      ? hoursAgoIso(24) : startIn;
    std::string end        = endIn.empty()        ? nowIso()         : endIn;
    std::string resolution = resolutionIn.empty() ? "auto"           : resolutionIn;

    // Auto resolution: estimate duration from ISO strings (simple comparison)
    // For simplicity, always use raw for <= 24 h requests when "auto" is requested.
    // A proper implementation would parse the datetimes; here we pass "auto"
    // through to the SQL time_bucket logic.

    auto conn = pool.acquire();
    pqxx::work txn(conn.get());
    pqxx::result rows;

    auto buildRow = [](const pqxx::row& r) {
        Json::Value j;
        auto ts = r[0];
        j["time"]             = ts.is_null()   ? Json::Value() : Json::Value(ts.as<std::string>());
        j["temperature"]      = r[1].is_null() ? Json::Value() : Json::Value(r[1].as<double>());
        j["dew_point"]        = r[2].is_null() ? Json::Value() : Json::Value(r[2].as<double>());
        j["humidity"]         = r[3].is_null() ? Json::Value() : Json::Value(r[3].as<double>());
        j["wind_speed"]       = r[4].is_null() ? Json::Value() : Json::Value(r[4].as<double>());
        j["wind_gust"]        = r[5].is_null() ? Json::Value() : Json::Value(r[5].as<double>());
        j["wind_direction"]   = r[6].is_null() ? Json::Value() : Json::Value(r[6].as<int>());
        j["precipitation_1h"] = r[7].is_null() ? Json::Value() : Json::Value(r[7].as<double>());
        j["snow_depth"]       = r[8].is_null() ? Json::Value() : Json::Value(r[8].as<double>());
        j["pressure"]         = r[9].is_null() ? Json::Value() : Json::Value(r[9].as<double>());
        j["visibility"]       = r[10].is_null()? Json::Value() : Json::Value(r[10].as<int>());
        j["cloud_cover"]      = r[11].is_null()? Json::Value() : Json::Value(r[11].as<int>());
        return j;
    };

    if (resolution == "raw") {
        rows = txn.exec_params(
            "SELECT "
            "  to_char(time AT TIME ZONE 'UTC', 'YYYY-MM-DD\"T\"HH24:MI:SS\"Z\"') AS time,"
            "  temperature, dew_point, humidity, wind_speed, wind_gust, wind_direction,"
            "  precipitation_1h, snow_depth, pressure, visibility, cloud_cover "
            "FROM observations "
            "WHERE fmisid=$1 AND time>=$2::timestamptz AND time<=$3::timestamptz "
            "ORDER BY time",
            fmisid, start, end
        );
        Json::Value arr(Json::arrayValue);
        for (const auto& r : rows) arr.append(buildRow(r));
        return arr;
    }

    // hourly / daily / auto via time_bucket
    std::string bucket = (resolution == "daily") ? "1 day" : "1 hour";
    try {
        rows = txn.exec_params(
            "SELECT "
            "  to_char(time_bucket($1::interval, time) AT TIME ZONE 'UTC', "
            "          'YYYY-MM-DD\"T\"HH24:MI:SS\"Z\"') AS time,"
            "  AVG(temperature), AVG(dew_point), AVG(humidity), "
            "  AVG(wind_speed), MAX(wind_gust), "
            "  ROUND(AVG(wind_direction))::int AS wind_direction,"
            "  SUM(precipitation_1h), AVG(snow_depth), AVG(pressure),"
            "  ROUND(AVG(visibility))::int AS visibility,"
            "  ROUND(AVG(cloud_cover))::int AS cloud_cover "
            "FROM observations "
            "WHERE fmisid=$2 AND time>=$3::timestamptz AND time<=$4::timestamptz "
            "GROUP BY 1 ORDER BY 1",
            bucket, fmisid, start, end
        );
        Json::Value arr(Json::arrayValue);
        for (const auto& r : rows) arr.append(buildRow(r));
        return arr;
    } catch (...) {
        // TimescaleDB not available – fall back to raw
        return getObservations(pool, fmisid, start, end, "raw");
    }
}

// ─── Latest per station ────────────────────────────────────────────────────

Json::Value getLatestObservations(ConnectionPool& pool,
                                  const std::vector<int>& fmisids)
{
    auto conn = pool.acquire();
    pqxx::work txn(conn.get());

    pqxx::result result;
    if (fmisids.empty()) {
        result = txn.exec(
            "SELECT s.fmisid, s.name, s.region, s.latitude, s.longitude, "
            "  to_char(o.time AT TIME ZONE 'UTC', 'YYYY-MM-DD\"T\"HH24:MI:SS\"Z\"') AS time,"
            "  o.temperature, o.humidity, o.wind_speed, o.wind_direction, o.wind_gust,"
            "  o.precipitation_1h, o.snow_depth, o.pressure "
            "FROM observations o "
            "JOIN stations s ON s.fmisid = o.fmisid "
            "JOIN ("
            "  SELECT fmisid, MAX(time) AS max_time FROM observations GROUP BY fmisid"
            ") latest ON o.fmisid = latest.fmisid AND o.time = latest.max_time "
            "ORDER BY s.name"
        );
    } else {
        std::string ids;
        for (size_t i = 0; i < fmisids.size(); ++i) {
            if (i) ids += ',';
            ids += std::to_string(fmisids[i]);
        }
        result = txn.exec(
            "SELECT s.fmisid, s.name, s.region, s.latitude, s.longitude, "
            "  to_char(o.time AT TIME ZONE 'UTC', 'YYYY-MM-DD\"T\"HH24:MI:SS\"Z\"') AS time,"
            "  o.temperature, o.humidity, o.wind_speed, o.wind_direction, o.wind_gust,"
            "  o.precipitation_1h, o.snow_depth, o.pressure "
            "FROM observations o "
            "JOIN stations s ON s.fmisid = o.fmisid "
            "JOIN ("
            "  SELECT fmisid, MAX(time) AS max_time FROM observations "
            "  WHERE fmisid IN (" + ids + ") GROUP BY fmisid"
            ") latest ON o.fmisid = latest.fmisid AND o.time = latest.max_time "
            "ORDER BY s.name"
        );
    }

    Json::Value arr(Json::arrayValue);
    for (const auto& r : result) {
        Json::Value j;
        j["fmisid"]           = r["fmisid"].as<int>();
        j["name"]             = r["name"].as<std::string>();
        j["region"]           = r["region"].is_null()           ? Json::Value() : Json::Value(r["region"].as<std::string>());
        j["latitude"]         = r["latitude"].as<double>();
        j["longitude"]        = r["longitude"].as<double>();
        j["time"]             = r["time"].is_null()             ? Json::Value() : Json::Value(r["time"].as<std::string>());
        j["temperature"]      = r["temperature"].is_null()      ? Json::Value() : Json::Value(r["temperature"].as<double>());
        j["humidity"]         = r["humidity"].is_null()         ? Json::Value() : Json::Value(r["humidity"].as<double>());
        j["wind_speed"]       = r["wind_speed"].is_null()       ? Json::Value() : Json::Value(r["wind_speed"].as<double>());
        j["wind_direction"]   = r["wind_direction"].is_null()   ? Json::Value() : Json::Value(r["wind_direction"].as<int>());
        j["wind_gust"]        = r["wind_gust"].is_null()        ? Json::Value() : Json::Value(r["wind_gust"].as<double>());
        j["precipitation_1h"] = r["precipitation_1h"].is_null() ? Json::Value() : Json::Value(r["precipitation_1h"].as<double>());
        j["snow_depth"]       = r["snow_depth"].is_null()       ? Json::Value() : Json::Value(r["snow_depth"].as<double>());
        j["pressure"]         = r["pressure"].is_null()         ? Json::Value() : Json::Value(r["pressure"].as<double>());
        arr.append(j);
    }
    return arr;
}

// ─── Latest single ─────────────────────────────────────────────────────────

std::optional<Observation> getLatestObservation(ConnectionPool& pool, int fmisid) {
    auto conn = pool.acquire();
    pqxx::work txn(conn.get());

    auto rows = txn.exec_params(
        "SELECT "
        "  to_char(time AT TIME ZONE 'UTC', 'YYYY-MM-DD\"T\"HH24:MI:SS\"Z\"') AS time,"
        "  fmisid, temperature, dew_point, humidity, wind_speed, wind_gust,"
        "  wind_direction, precipitation_1h, precip_intensity, snow_depth,"
        "  pressure, visibility, cloud_cover, weather_code "
        "FROM observations WHERE fmisid=$1 ORDER BY time DESC LIMIT 1",
        fmisid
    );

    if (rows.empty()) return std::nullopt;
    const auto& r = rows[0];
    Observation o;
    o.time   = r["time"].is_null() ? "" : r["time"].as<std::string>();
    o.fmisid = r["fmisid"].as<int>();
    if (!r["temperature"].is_null())     o.temperature     = r["temperature"].as<double>();
    if (!r["dew_point"].is_null())       o.dew_point       = r["dew_point"].as<double>();
    if (!r["humidity"].is_null())        o.humidity        = r["humidity"].as<double>();
    if (!r["wind_speed"].is_null())      o.wind_speed      = r["wind_speed"].as<double>();
    if (!r["wind_gust"].is_null())       o.wind_gust       = r["wind_gust"].as<double>();
    if (!r["wind_direction"].is_null())  o.wind_direction  = r["wind_direction"].as<int>();
    if (!r["precipitation_1h"].is_null()) o.precipitation_1h = r["precipitation_1h"].as<double>();
    if (!r["precip_intensity"].is_null()) o.precip_intensity = r["precip_intensity"].as<double>();
    if (!r["snow_depth"].is_null())      o.snow_depth      = r["snow_depth"].as<double>();
    if (!r["pressure"].is_null())        o.pressure        = r["pressure"].as<double>();
    if (!r["visibility"].is_null())      o.visibility      = r["visibility"].as<int>();
    if (!r["cloud_cover"].is_null())     o.cloud_cover     = r["cloud_cover"].as<int>();
    if (!r["weather_code"].is_null())    o.weather_code    = r["weather_code"].as<int>();
    return o;
}

// ─── Summary ───────────────────────────────────────────────────────────────

Json::Value getSummary(ConnectionPool& pool,
                       int fmisid,
                       const std::string& period)
{
    int hours = 24;
    if      (period == "7d")  hours = 168;
    else if (period == "30d") hours = 720;

    auto conn = pool.acquire();
    pqxx::work txn(conn.get());

    auto rows = txn.exec_params(
        "SELECT "
        "  MIN(temperature)      AS min_temperature,"
        "  MAX(temperature)      AS max_temperature,"
        "  AVG(temperature)      AS avg_temperature,"
        "  AVG(humidity)         AS avg_humidity,"
        "  AVG(wind_speed)       AS avg_wind_speed,"
        "  MAX(wind_gust)        AS max_wind_gust,"
        "  SUM(precipitation_1h) AS total_precipitation,"
        "  AVG(pressure)         AS avg_pressure "
        "FROM observations "
        "WHERE fmisid=$1 AND time >= NOW() - ($2 * INTERVAL '1 hour')",
        fmisid, hours
    );

    Json::Value j;
    j["fmisid"] = fmisid;
    j["period"] = period;
    if (!rows.empty()) {
        const auto& r = rows[0];
        j["min_temperature"]    = r["min_temperature"].is_null()    ? Json::Value() : Json::Value(r["min_temperature"].as<double>());
        j["max_temperature"]    = r["max_temperature"].is_null()    ? Json::Value() : Json::Value(r["max_temperature"].as<double>());
        j["avg_temperature"]    = r["avg_temperature"].is_null()    ? Json::Value() : Json::Value(r["avg_temperature"].as<double>());
        j["avg_humidity"]       = r["avg_humidity"].is_null()       ? Json::Value() : Json::Value(r["avg_humidity"].as<double>());
        j["avg_wind_speed"]     = r["avg_wind_speed"].is_null()     ? Json::Value() : Json::Value(r["avg_wind_speed"].as<double>());
        j["max_wind_gust"]      = r["max_wind_gust"].is_null()      ? Json::Value() : Json::Value(r["max_wind_gust"].as<double>());
        j["total_precipitation"]= r["total_precipitation"].is_null()? Json::Value() : Json::Value(r["total_precipitation"].as<double>());
        j["avg_pressure"]       = r["avg_pressure"].is_null()       ? Json::Value() : Json::Value(r["avg_pressure"].as<double>());
    }
    return j;
}

} // namespace weather::services
