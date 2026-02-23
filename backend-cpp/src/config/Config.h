#pragma once

#include <string>
#include <vector>

namespace weather {

struct Config {
    std::string db_host;
    int         db_port{5432};
    std::string db_name;
    std::string db_user;
    std::string db_password;

    std::string fmi_bbox;
    int         fetch_interval_minutes{10};
    int         forecast_interval_minutes{60};
    std::vector<std::string> default_places;

    int server_port{8001};

    static Config fromEnv();
    std::string connString() const;
};

// Global config accessors (initialised in main)
void   initConfig(const Config& c);
const Config& getConfig();

} // namespace weather
