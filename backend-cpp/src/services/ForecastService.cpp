#include "ForecastService.h"

#include <pqxx/pqxx>

namespace weather::services {

void upsertForecasts(ConnectionPool& pool,
                     const std::vector<Forecast>& forecasts)
{
    if (forecasts.empty()) return;
    auto conn = pool.acquire();
    pqxx::work txn(conn.get());

    for (const auto& f : forecasts) {
        txn.exec_params(
            "INSERT INTO forecasts "
            "(fetched_at, valid_time, fmisid, place_name, latitude, longitude, model,"
            " temperature, wind_speed, wind_direction, wind_gust, precipitation_1h,"
            " humidity, pressure, cloud_cover, dew_point, weather_symbol) "
            "VALUES ($1,$2,$3,$4,$5,$6,$7,$8,$9,$10,$11,$12,$13,$14,$15,$16,$17)",
            f.fetched_at,
            f.valid_time,
            f.fmisid,
            f.place_name,
            f.latitude,
            f.longitude,
            f.model,
            f.temperature,
            f.wind_speed,
            f.wind_direction,
            f.wind_gust,
            f.precipitation_1h,
            f.humidity,
            f.pressure,
            f.cloud_cover,
            f.dew_point,
            f.weather_symbol
        );
    }
    txn.commit();
}

static std::vector<Forecast> resultToForecasts(const pqxx::result& rows) {
    std::vector<Forecast> result;
    for (const auto& r : rows) {
        Forecast f;
        f.id         = r["id"].as<long long>();
        f.fetched_at = r["fetched_at"].is_null() ? "" : r["fetched_at"].as<std::string>();
        f.valid_time = r["valid_time"].is_null()  ? "" : r["valid_time"].as<std::string>();
        if (!r["fmisid"].is_null())      f.fmisid      = r["fmisid"].as<int>();
        if (!r["place_name"].is_null())  f.place_name  = r["place_name"].as<std::string>();
        if (!r["latitude"].is_null())    f.latitude    = r["latitude"].as<double>();
        if (!r["longitude"].is_null())   f.longitude   = r["longitude"].as<double>();
        f.model = r["model"].as<std::string>();
        if (!r["temperature"].is_null())     f.temperature     = r["temperature"].as<double>();
        if (!r["wind_speed"].is_null())      f.wind_speed      = r["wind_speed"].as<double>();
        if (!r["wind_direction"].is_null())  f.wind_direction  = r["wind_direction"].as<int>();
        if (!r["wind_gust"].is_null())       f.wind_gust       = r["wind_gust"].as<double>();
        if (!r["precipitation_1h"].is_null()) f.precipitation_1h = r["precipitation_1h"].as<double>();
        if (!r["humidity"].is_null())        f.humidity        = r["humidity"].as<double>();
        if (!r["pressure"].is_null())        f.pressure        = r["pressure"].as<double>();
        if (!r["cloud_cover"].is_null())     f.cloud_cover     = r["cloud_cover"].as<double>();
        if (!r["dew_point"].is_null())       f.dew_point       = r["dew_point"].as<double>();
        if (!r["weather_symbol"].is_null())  f.weather_symbol  = r["weather_symbol"].as<int>();
        result.push_back(std::move(f));
    }
    return result;
}

static const char* SELECT_COLS =
    "SELECT id, "
    "  to_char(fetched_at AT TIME ZONE 'UTC', 'YYYY-MM-DD\"T\"HH24:MI:SS\"Z\"') AS fetched_at,"
    "  to_char(valid_time  AT TIME ZONE 'UTC', 'YYYY-MM-DD\"T\"HH24:MI:SS\"Z\"') AS valid_time,"
    "  fmisid, place_name, latitude, longitude, model,"
    "  temperature, wind_speed, wind_direction, wind_gust, precipitation_1h,"
    "  humidity, pressure, cloud_cover, dew_point, weather_symbol "
    "FROM forecasts ";

std::vector<Forecast> getForecastsByFmisid(ConnectionPool& pool,
                                            int fmisid,
                                            const std::string& model)
{
    auto conn = pool.acquire();
    pqxx::work txn(conn.get());

    // Find latest fetched_at
    auto latestRows = txn.exec_params(
        "SELECT MAX(fetched_at) FROM forecasts WHERE fmisid=$1 AND model=$2",
        fmisid, model
    );
    if (latestRows.empty() || latestRows[0][0].is_null()) return {};
    std::string latestAt = latestRows[0][0].as<std::string>();

    auto rows = txn.exec_params(
        std::string(SELECT_COLS) +
        "WHERE fmisid=$1 AND model=$2 AND fetched_at=$3::timestamptz "
        "  AND valid_time >= NOW() "
        "ORDER BY valid_time",
        fmisid, model, latestAt
    );
    return resultToForecasts(rows);
}

std::vector<Forecast> getForecastsByPlace(ConnectionPool& pool,
                                           const std::string& place,
                                           const std::string& model)
{
    auto conn = pool.acquire();
    pqxx::work txn(conn.get());

    auto latestRows = txn.exec_params(
        "SELECT MAX(fetched_at) FROM forecasts WHERE place_name=$1 AND model=$2",
        place, model
    );
    if (latestRows.empty() || latestRows[0][0].is_null()) return {};
    std::string latestAt = latestRows[0][0].as<std::string>();

    auto rows = txn.exec_params(
        std::string(SELECT_COLS) +
        "WHERE place_name=$1 AND model=$2 AND fetched_at=$3::timestamptz "
        "  AND valid_time >= NOW() "
        "ORDER BY valid_time",
        place, model, latestAt
    );
    return resultToForecasts(rows);
}

} // namespace weather::services
