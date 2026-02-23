#pragma once

#include <json/json.h>
#include <optional>
#include <string>

namespace weather {

struct Station {
    int         fmisid{0};
    std::string name;
    std::optional<std::string> region;
    std::string country{"Finland"};
    double      latitude{0.0};
    double      longitude{0.0};
    std::optional<double> elevation;
    std::optional<std::string> station_type;
    bool        is_active{true};
    std::string created_at;
    std::string updated_at;

    Json::Value toJson() const {
        Json::Value j;
        j["fmisid"]       = fmisid;
        j["name"]         = name;
        j["region"]       = region    ? Json::Value(*region)    : Json::Value();
        j["country"]      = country;
        j["latitude"]     = latitude;
        j["longitude"]    = longitude;
        j["elevation"]    = elevation ? Json::Value(*elevation) : Json::Value();
        j["station_type"] = station_type ? Json::Value(*station_type) : Json::Value();
        j["is_active"]    = is_active;
        j["created_at"]   = created_at;
        j["updated_at"]   = updated_at;
        return j;
    }
};

} // namespace weather
