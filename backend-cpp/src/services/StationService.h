#pragma once

#include "../db/ConnectionPool.h"
#include "../models/Station.h"

#include <optional>
#include <string>
#include <vector>

namespace weather::services {

struct StationMeta {
    int         fmisid;
    std::string name;
    double      latitude;
    double      longitude;
    std::optional<std::string> region;
    std::optional<std::string> country;
    std::optional<double>      elevation;
};

void upsertStations(ConnectionPool& pool, const std::vector<StationMeta>& stations);

std::vector<Station>     getStations(ConnectionPool& pool, bool activeOnly);
std::optional<Station>   getStation(ConnectionPool& pool, int fmisid);

// Returns station + latest observation fields merged into one JSON object
Json::Value getStationWithLatest(ConnectionPool& pool, int fmisid);

} // namespace weather::services
