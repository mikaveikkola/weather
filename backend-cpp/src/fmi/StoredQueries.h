#pragma once

#include <map>
#include <set>
#include <string>

namespace weather::fmi {

inline const std::string FMI_WFS_BASE =
    "https://opendata.fmi.fi/wfs";

inline const std::string OBSERVATIONS_MULTIPOINTCOVERAGE =
    "fmi::observations::weather::multipointcoverage";

inline const std::string FORECAST_HARMONIE =
    "fmi::forecast::harmonie::surface::point::multipointcoverage";

// Observations: WFS swe:field name → DB column
// (FMI WFS uses short technical codes in multipointcoverage)
inline const std::map<std::string, std::string> OBS_PARAM_MAP = {
    {"t2m",      "temperature"},
    {"td",       "dew_point"},
    {"rh",       "humidity"},
    {"ws_10min", "wind_speed"},
    {"wg_10min", "wind_gust"},
    {"wd_10min", "wind_direction"},
    {"r_1h",     "precipitation_1h"},
    {"ri_10min", "precip_intensity"},
    {"snow_aws", "snow_depth"},
    {"p_sea",    "pressure"},
    {"vis",      "visibility"},
    {"n_man",    "cloud_cover"},
    {"wawa",     "weather_code"},
};

// Forecasts: WFS swe:field name → DB column
// (FMI Harmonie WFS uses CamelCase names)
inline const std::map<std::string, std::string> FORECAST_PARAM_MAP = {
    {"Temperature",        "temperature"},
    {"DewPoint",           "dew_point"},
    {"Humidity",           "humidity"},
    {"WindDirection",      "wind_direction"},
    {"WindSpeedMS",        "wind_speed"},
    {"WindGust",           "wind_gust"},
    {"PrecipitationAmount","precipitation_1h"},
    {"Pressure",           "pressure"},
    {"TotalCloudCover",    "cloud_cover"},
    {"WeatherSymbol3",     "weather_symbol"},
};

// Fields that store integers (need rounding)
inline const std::set<std::string> INT_OBS_FIELDS = {
    "wind_direction", "cloud_cover", "weather_code", "visibility"
};

inline const std::set<std::string> INT_FORECAST_FIELDS = {
    "wind_direction", "weather_symbol"
};

} // namespace weather::fmi
