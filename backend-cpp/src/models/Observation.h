#pragma once

#include <json/json.h>
#include <optional>
#include <string>

namespace weather {

struct Observation {
    std::string time;   // ISO 8601 UTC
    int         fmisid{0};

    std::optional<double> temperature;
    std::optional<double> dew_point;
    std::optional<double> humidity;
    std::optional<double> wind_speed;
    std::optional<double> wind_gust;
    std::optional<int>    wind_direction;
    std::optional<double> precipitation_1h;
    std::optional<double> precip_intensity;
    std::optional<double> snow_depth;
    std::optional<double> pressure;
    std::optional<int>    visibility;
    std::optional<int>    cloud_cover;
    std::optional<int>    weather_code;

    Json::Value toJson() const {
        Json::Value j;
        j["time"]              = time;
        j["fmisid"]            = fmisid;
        j["temperature"]       = temperature      ? Json::Value(*temperature)      : Json::Value();
        j["dew_point"]         = dew_point        ? Json::Value(*dew_point)        : Json::Value();
        j["humidity"]          = humidity         ? Json::Value(*humidity)         : Json::Value();
        j["wind_speed"]        = wind_speed       ? Json::Value(*wind_speed)       : Json::Value();
        j["wind_gust"]         = wind_gust        ? Json::Value(*wind_gust)        : Json::Value();
        j["wind_direction"]    = wind_direction   ? Json::Value(*wind_direction)   : Json::Value();
        j["precipitation_1h"]  = precipitation_1h ? Json::Value(*precipitation_1h) : Json::Value();
        j["precip_intensity"]  = precip_intensity ? Json::Value(*precip_intensity) : Json::Value();
        j["snow_depth"]        = snow_depth       ? Json::Value(*snow_depth)       : Json::Value();
        j["pressure"]          = pressure         ? Json::Value(*pressure)         : Json::Value();
        j["visibility"]        = visibility       ? Json::Value(*visibility)       : Json::Value();
        j["cloud_cover"]       = cloud_cover      ? Json::Value(*cloud_cover)      : Json::Value();
        j["weather_code"]      = weather_code     ? Json::Value(*weather_code)     : Json::Value();
        return j;
    }
};

} // namespace weather
