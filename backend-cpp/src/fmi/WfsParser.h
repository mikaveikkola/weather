#pragma once

#include "../models/Observation.h"
#include "../models/Forecast.h"
#include "../services/StationService.h"

#include <optional>
#include <string>
#include <vector>

namespace weather {

struct ParsedObservations {
    std::vector<services::StationMeta> stations;
    std::vector<Observation>           observations;
};

struct ParsedForecasts {
    std::optional<services::StationMeta> station;
    std::vector<Forecast>                forecasts;
};

class WfsParser {
public:
    ParsedObservations parseObservations(const std::string& xml);

    ParsedForecasts parseForecasts(const std::string& xml,
                                   const std::string& place,
                                   const std::string& fetchedAt);
};

} // namespace weather
