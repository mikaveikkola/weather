#pragma once

#include "../db/ConnectionPool.h"
#include "../models/Observation.h"

#include <json/json.h>
#include <optional>
#include <string>
#include <vector>

namespace weather::services {

void upsertObservations(ConnectionPool& pool,
                        const std::vector<Observation>& observations);

// Returns observations as JSON array; resolution: "raw"|"hourly"|"daily"|"auto"
Json::Value getObservations(ConnectionPool& pool,
                            int fmisid,
                            const std::string& start,   // ISO 8601
                            const std::string& end,
                            const std::string& resolution);

// Returns latest observation per station, joined with station name/region
Json::Value getLatestObservations(ConnectionPool& pool,
                                  const std::vector<int>& fmisids);

// Returns latest observation for a single station
std::optional<Observation> getLatestObservation(ConnectionPool& pool, int fmisid);

// Returns summary statistics
Json::Value getSummary(ConnectionPool& pool,
                       int fmisid,
                       const std::string& period);  // "24h"|"7d"|"30d"

} // namespace weather::services
