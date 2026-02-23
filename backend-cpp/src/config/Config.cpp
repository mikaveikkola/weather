#include "Config.h"

#include <cstdlib>
#include <sstream>

namespace weather {

static Config g_config;

static std::string getenv_or(const char* key, const char* def) {
    const char* val = std::getenv(key);
    return val ? std::string(val) : std::string(def);
}

static int getenv_int(const char* key, int def) {
    const char* val = std::getenv(key);
    if (!val) return def;
    try { return std::stoi(val); } catch (...) { return def; }
}

static std::vector<std::string> split(const std::string& s, char delim) {
    std::vector<std::string> result;
    std::istringstream iss(s);
    std::string token;
    while (std::getline(iss, token, delim)) {
        if (!token.empty()) result.push_back(token);
    }
    return result;
}

Config Config::fromEnv() {
    Config c;
    c.db_host     = getenv_or("DB_HOST",     "localhost");
    c.db_port     = getenv_int("DB_PORT",     5432);
    c.db_name     = getenv_or("DB_NAME",     "weather");
    c.db_user     = getenv_or("DB_USER",     "weather");
    c.db_password = getenv_or("DB_PASSWORD", "weather");

    c.fmi_bbox                  = getenv_or("FMI_BBOX", "19,59,32,71");
    c.fetch_interval_minutes    = getenv_int("FETCH_INTERVAL_MINUTES",    10);
    c.forecast_interval_minutes = getenv_int("FORECAST_INTERVAL_MINUTES", 60);

    std::string places_str = getenv_or("DEFAULT_PLACES",
                                       "Helsinki,Tampere,Turku,Oulu,Rovaniemi");
    c.default_places = split(places_str, ',');
    c.server_port = 8001;
    return c;
}

std::string Config::connString() const {
    std::ostringstream oss;
    oss << "host="     << db_host
        << " port="    << db_port
        << " dbname="  << db_name
        << " user="    << db_user
        << " password=" << db_password
        << " connect_timeout=10";
    return oss.str();
}

void initConfig(const Config& c) {
    g_config = c;
}

const Config& getConfig() {
    return g_config;
}

} // namespace weather
