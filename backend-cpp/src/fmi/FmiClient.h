#pragma once

#include <string>

namespace weather::fmi {

struct FetchResult {
    bool        success{false};
    std::string xml;
    std::string error;
};

class FmiClient {
public:
    // Fetch multipointcoverage observations
    FetchResult fetchObservations(const std::string& starttime,   // ISO 8601 UTC
                                  const std::string& endtime,
                                  const std::string& bbox);

    // Fetch Harmonie surface point forecast for a named place
    FetchResult fetchForecastHarmonie(const std::string& place);

private:
    FetchResult doGet(const std::string& url);
};

} // namespace weather::fmi
