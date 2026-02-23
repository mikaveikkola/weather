#include "StationService.h"

#include <pqxx/pqxx>
#include <stdexcept>

namespace weather::services {

static Station rowToStation(const pqxx::row& row) {
    Station s;
    s.fmisid   = row["fmisid"].as<int>();
    s.name     = row["name"].as<std::string>();
    s.region   = row["region"].is_null()
                     ? std::nullopt
                     : std::make_optional(row["region"].as<std::string>());
    s.country  = row["country"].is_null() ? "Finland" : row["country"].as<std::string>();
    s.latitude  = row["latitude"].as<double>();
    s.longitude = row["longitude"].as<double>();
    s.elevation = row["elevation"].is_null()
                      ? std::nullopt
                      : std::make_optional(row["elevation"].as<double>());
    s.station_type = row["station_type"].is_null()
                         ? std::nullopt
                         : std::make_optional(row["station_type"].as<std::string>());
    s.is_active  = row["is_active"].as<bool>();
    s.created_at = row["created_at"].is_null() ? "" : row["created_at"].as<std::string>();
    s.updated_at = row["updated_at"].is_null() ? "" : row["updated_at"].as<std::string>();
    return s;
}

void upsertStations(ConnectionPool& pool, const std::vector<StationMeta>& stations) {
    if (stations.empty()) return;
    auto conn = pool.acquire();
    pqxx::work txn(conn.get());

    for (const auto& s : stations) {
        txn.exec_params(
            "INSERT INTO stations "
            "  (fmisid, name, region, country, latitude, longitude, elevation, station_type, is_active) "
            "VALUES ($1,$2,$3,$4,$5,$6,$7,$8,TRUE) "
            "ON CONFLICT (fmisid) DO UPDATE SET "
            "  name=EXCLUDED.name, region=EXCLUDED.region, "
            "  latitude=EXCLUDED.latitude, longitude=EXCLUDED.longitude, "
            "  elevation=EXCLUDED.elevation, updated_at=NOW()",
            s.fmisid,
            s.name,
            s.region,
            s.country.value_or("Finland"),
            s.latitude,
            s.longitude,
            s.elevation,
            std::string("weather")
        );
    }
    txn.commit();
}

std::vector<Station> getStations(ConnectionPool& pool, bool activeOnly) {
    auto conn = pool.acquire();
    pqxx::work txn(conn.get());

    pqxx::result rows;
    if (activeOnly) {
        rows = txn.exec(
            "SELECT fmisid, name, region, country, latitude, longitude, elevation, "
            "  station_type, is_active, "
            "  to_char(created_at AT TIME ZONE 'UTC', 'YYYY-MM-DD\"T\"HH24:MI:SS\"Z\"') AS created_at, "
            "  to_char(updated_at AT TIME ZONE 'UTC', 'YYYY-MM-DD\"T\"HH24:MI:SS\"Z\"') AS updated_at "
            "FROM stations WHERE is_active=TRUE ORDER BY name"
        );
    } else {
        rows = txn.exec(
            "SELECT fmisid, name, region, country, latitude, longitude, elevation, "
            "  station_type, is_active, "
            "  to_char(created_at AT TIME ZONE 'UTC', 'YYYY-MM-DD\"T\"HH24:MI:SS\"Z\"') AS created_at, "
            "  to_char(updated_at AT TIME ZONE 'UTC', 'YYYY-MM-DD\"T\"HH24:MI:SS\"Z\"') AS updated_at "
            "FROM stations ORDER BY name"
        );
    }

    std::vector<Station> result;
    for (const auto& row : rows)
        result.push_back(rowToStation(row));
    return result;
}

std::optional<Station> getStation(ConnectionPool& pool, int fmisid) {
    auto conn = pool.acquire();
    pqxx::work txn(conn.get());

    auto rows = txn.exec_params(
        "SELECT fmisid, name, region, country, latitude, longitude, elevation, "
        "  station_type, is_active, "
        "  to_char(created_at AT TIME ZONE 'UTC', 'YYYY-MM-DD\"T\"HH24:MI:SS\"Z\"') AS created_at, "
        "  to_char(updated_at AT TIME ZONE 'UTC', 'YYYY-MM-DD\"T\"HH24:MI:SS\"Z\"') AS updated_at "
        "FROM stations WHERE fmisid=$1",
        fmisid
    );

    if (rows.empty()) return std::nullopt;
    return rowToStation(rows[0]);
}

Json::Value getStationWithLatest(ConnectionPool& pool, int fmisid) {
    auto conn = pool.acquire();
    pqxx::work txn(conn.get());

    // Station info
    auto srows = txn.exec_params(
        "SELECT fmisid, name, region, country, latitude, longitude, elevation, "
        "  station_type, is_active "
        "FROM stations WHERE fmisid=$1",
        fmisid
    );
    if (srows.empty()) return Json::Value();

    const auto& sr = srows[0];
    Json::Value j;
    j["fmisid"]       = sr["fmisid"].as<int>();
    j["name"]         = sr["name"].as<std::string>();
    j["region"]       = sr["region"].is_null()       ? Json::Value() : Json::Value(sr["region"].as<std::string>());
    j["country"]      = sr["country"].is_null()      ? Json::Value("Finland") : Json::Value(sr["country"].as<std::string>());
    j["latitude"]     = sr["latitude"].as<double>();
    j["longitude"]    = sr["longitude"].as<double>();
    j["is_active"]    = sr["is_active"].as<bool>();

    // Latest observation
    auto orows = txn.exec_params(
        "SELECT "
        "  to_char(time AT TIME ZONE 'UTC', 'YYYY-MM-DD\"T\"HH24:MI:SS\"Z\"') AS time, "
        "  temperature, humidity, wind_speed, wind_direction, wind_gust, "
        "  precipitation_1h, snow_depth, pressure "
        "FROM observations WHERE fmisid=$1 ORDER BY time DESC LIMIT 1",
        fmisid
    );

    if (!orows.empty()) {
        const auto& or_ = orows[0];
        j["time"]             = or_["time"].is_null()             ? Json::Value() : Json::Value(or_["time"].as<std::string>());
        j["temperature"]      = or_["temperature"].is_null()      ? Json::Value() : Json::Value(or_["temperature"].as<double>());
        j["humidity"]         = or_["humidity"].is_null()         ? Json::Value() : Json::Value(or_["humidity"].as<double>());
        j["wind_speed"]       = or_["wind_speed"].is_null()       ? Json::Value() : Json::Value(or_["wind_speed"].as<double>());
        j["wind_direction"]   = or_["wind_direction"].is_null()   ? Json::Value() : Json::Value(or_["wind_direction"].as<int>());
        j["wind_gust"]        = or_["wind_gust"].is_null()        ? Json::Value() : Json::Value(or_["wind_gust"].as<double>());
        j["precipitation_1h"] = or_["precipitation_1h"].is_null() ? Json::Value() : Json::Value(or_["precipitation_1h"].as<double>());
        j["snow_depth"]       = or_["snow_depth"].is_null()       ? Json::Value() : Json::Value(or_["snow_depth"].as<double>());
        j["pressure"]         = or_["pressure"].is_null()         ? Json::Value() : Json::Value(or_["pressure"].as<double>());
    } else {
        j["time"] = Json::Value();
        j["temperature"] = j["humidity"] = j["wind_speed"] = j["wind_direction"] =
        j["wind_gust"] = j["precipitation_1h"] = j["snow_depth"] = j["pressure"] = Json::Value();
    }
    return j;
}

} // namespace weather::services
