#pragma once

#include <json/json.h>
#include <optional>
#include <string>

namespace weather {

struct Forecast {
    long long   id{0};
    std::string fetched_at;   // ISO 8601 UTC
    std::string valid_time;   // ISO 8601 UTC
    std::optional<int>    fmisid;
    std::optional<std::string> place_name;
    std::optional<double> latitude;
    std::optional<double> longitude;
    std::string model;

    std::optional<double> temperature;
    std::optional<double> wind_speed;
    std::optional<int>    wind_direction;
    std::optional<double> wind_gust;
    std::optional<double> precipitation_1h;
    std::optional<double> humidity;
    std::optional<double> pressure;
    std::optional<double> cloud_cover;
    std::optional<double> dew_point;
    std::optional<int>    weather_symbol;

    Json::Value toJson() const {
        Json::Value j;
        j["id"]               = static_cast<Json::Int64>(id);
        j["fetched_at"]       = fetched_at;
        j["valid_time"]       = valid_time;
        j["fmisid"]           = fmisid      ? Json::Value(*fmisid)      : Json::Value();
        j["place_name"]       = place_name  ? Json::Value(*place_name)  : Json::Value();
        j["model"]            = model;
        j["temperature"]      = temperature     ? Json::Value(*temperature)     : Json::Value();
        j["wind_speed"]       = wind_speed      ? Json::Value(*wind_speed)      : Json::Value();
        j["wind_direction"]   = wind_direction  ? Json::Value(*wind_direction)  : Json::Value();
        j["wind_gust"]        = wind_gust       ? Json::Value(*wind_gust)       : Json::Value();
        j["precipitation_1h"] = precipitation_1h ? Json::Value(*precipitation_1h) : Json::Value();
        j["humidity"]         = humidity        ? Json::Value(*humidity)        : Json::Value();
        j["pressure"]         = pressure        ? Json::Value(*pressure)        : Json::Value();
        j["cloud_cover"]      = cloud_cover     ? Json::Value(*cloud_cover)     : Json::Value();
        j["dew_point"]        = dew_point       ? Json::Value(*dew_point)       : Json::Value();
        j["weather_symbol"]   = weather_symbol  ? Json::Value(*weather_symbol)  : Json::Value();
        return j;
    }
};

} // namespace weather
