#pragma once

#include "../db/ConnectionPool.h"
#include "../models/Forecast.h"

#include <json/json.h>
#include <string>
#include <vector>

namespace weather::services {

void upsertForecasts(ConnectionPool& pool,
                     const std::vector<Forecast>& forecasts);

// Returns latest forecast batch by fmisid
std::vector<Forecast> getForecastsByFmisid(ConnectionPool& pool,
                                            int fmisid,
                                            const std::string& model);

// Returns latest forecast batch by place name
std::vector<Forecast> getForecastsByPlace(ConnectionPool& pool,
                                           const std::string& place,
                                           const std::string& model);

// Returns past forecasts (most recent fetch per valid_time) for obs comparison
std::vector<Forecast> getForecastsHistoryByPlace(ConnectionPool& pool,
                                                  const std::string& place,
                                                  const std::string& model,
                                                  int hours);

} // namespace weather::services
