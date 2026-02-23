# FMI WFS stored query IDs
OBSERVATIONS_MULTIPOINTCOVERAGE = "fmi::observations::weather::multipointcoverage"
OBSERVATIONS_HOURLY = "fmi::observations::weather::hourly::multipointcoverage"
OBSERVATIONS_DAILY = "fmi::observations::weather::daily::multipointcoverage"
FORECAST_HARMONIE = "fmi::forecast::harmonie::surface::point::multipointcoverage"
FORECAST_ECMWF = "ecmwf::forecast::surface::point::timevaluepair"

# Parameter name mapping: fmiopendata long name -> our field name
OBS_PARAM_MAP = {
    "Air temperature": "temperature",
    "Dew-point temperature": "dew_point",
    "Relative humidity": "humidity",
    "Wind speed": "wind_speed",
    "Gust speed": "wind_gust",
    "Wind direction": "wind_direction",
    "Precipitation amount": "precipitation_1h",
    "Precipitation intensity": "precip_intensity",
    "Snow depth": "snow_depth",
    "Pressure (msl)": "pressure",
    "Horizontal visibility": "visibility",
    "Cloud amount": "cloud_cover",
    "Present weather (auto)": "weather_code",
}

FORECAST_PARAM_MAP = {
    "Air temperature": "temperature",
    "Wind speed": "wind_speed",
    "Wind direction": "wind_direction",
    "Wind gust": "wind_gust",
    "Precipitation amount": "precipitation_1h",
    "Humidity": "humidity",
    "Air pressure": "pressure",
    "Total cloud cover": "cloud_cover",
    "Dew point": "dew_point",
}
