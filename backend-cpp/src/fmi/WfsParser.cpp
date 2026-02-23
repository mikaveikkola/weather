#include "WfsParser.h"
#include "StoredQueries.h"

#include <drogon/drogon.h>
#include <pugixml.hpp>
#include <cmath>
#include <ctime>
#include <map>
#include <sstream>
#include <string_view>

namespace weather {

// ─── Helpers ───────────────────────────────────────────────────────────────

static std::string_view localName(const char* nodeName) {
    std::string_view sv(nodeName);
    auto pos = sv.rfind(':');
    return (pos != std::string_view::npos) ? sv.substr(pos + 1) : sv;
}

static std::string epochToIso(long long epoch) {
    time_t t = static_cast<time_t>(epoch);
    struct tm utc{};
    gmtime_r(&t, &utc);
    char buf[32];
    strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &utc);
    return buf;
}

static std::optional<double> safeDouble(const std::string& s) {
    if (s.empty() || s == "NaN") return std::nullopt;
    try {
        double v = std::stod(s);
        if (std::isnan(v) || std::isinf(v)) return std::nullopt;
        return v;
    } catch (...) {
        return std::nullopt;
    }
}

// ─── Tree-walking helpers ──────────────────────────────────────────────────
// pugixml's XPath local-name() returns the full prefixed name, not just the
// local part. Use these C++ tree-walkers with the localName() helper instead.

// Find first descendant with given local name (depth-first)
static pugi::xml_node findFirst(const pugi::xml_node& node, const char* lname) {
    for (const auto& child : node.children()) {
        if (localName(child.name()) == lname) return child;
        auto f = findFirst(child, lname);
        if (f) return f;
    }
    return {};
}

// Collect all descendants with given local name
static void findAll(const pugi::xml_node& node, const char* lname,
                    std::vector<pugi::xml_node>& out) {
    for (const auto& child : node.children()) {
        if (localName(child.name()) == lname) out.push_back(child);
        findAll(child, lname, out);
    }
}

// ─── Extract field names ───────────────────────────────────────────────────

static std::vector<std::string> extractFieldNames(const pugi::xml_document& doc) {
    std::vector<std::string> fields;
    // Find the swe:DataRecord that lists coverage fields
    auto dr = findFirst(doc, "DataRecord");
    if (!dr) return fields;
    // Direct children named swe:field carry the parameter names
    for (const auto& child : dr.children()) {
        if (localName(child.name()) == "field") {
            auto attr = child.attribute("name");
            if (attr) fields.emplace_back(attr.value());
        }
    }
    return fields;
}

// ─── Extract (lat, lon, epoch) triples ────────────────────────────────────

struct Position { double lat, lon; long long epoch; };

static std::vector<Position> extractPositions(const pugi::xml_document& doc) {
    std::vector<Position> result;
    auto node = findFirst(doc, "positions");
    if (!node) return result;
    std::istringstream iss(node.text().get());
    double lat, lon;
    long long epoch;
    while (iss >> lat >> lon >> epoch)
        result.push_back({lat, lon, epoch});
    return result;
}

// ─── Extract values ────────────────────────────────────────────────────────

static std::vector<std::optional<double>> extractValues(const pugi::xml_document& doc) {
    auto node = findFirst(doc, "doubleOrNilReasonTupleList");
    if (!node) return {};
    std::vector<std::optional<double>> result;
    std::istringstream iss(node.text().get());
    std::string token;
    while (iss >> token)
        result.push_back(safeDouble(token));
    return result;
}

// ─── Extract station metadata from Location elements ─────────────────────

static std::vector<services::StationMeta> extractStations(const pugi::xml_document& doc) {
    std::vector<services::StationMeta> stations;
    std::map<int, bool> seen;

    std::vector<pugi::xml_node> locNodes;
    findAll(doc, "Location", locNodes);

    for (const auto& loc : locNodes) {
        services::StationMeta meta;
        bool hasId   = false;
        bool hasName = false;

        for (const auto& child : loc.children()) {
            std::string_view ln = localName(child.name());

            if (ln == "identifier") {
                std::string id = child.text().get();
                auto pos = id.rfind('/');
                if (pos != std::string::npos) {
                    try {
                        meta.fmisid = std::stoi(id.substr(pos + 1));
                        hasId = true;
                    } catch (...) {}
                }
            } else if (ln == "name") {
                if (!hasName && child.text().get()[0] != '\0') {
                    meta.name = child.text().get();
                    hasName = true;
                }
            } else if (ln == "region") {
                std::string r = child.text().get();
                if (!r.empty()) meta.region = r;
            } else if (ln == "country") {
                std::string c = child.text().get();
                if (!c.empty()) meta.country = c;
            } else if (ln == "representativePoint") {
                // gml:Point/gml:pos contains "lat lon"
                for (const auto& gc : child.children()) {
                    for (const auto& ggc : gc.children()) {
                        if (localName(ggc.name()) == "pos") {
                            std::istringstream iss(ggc.text().get());
                            iss >> meta.latitude >> meta.longitude;
                        }
                    }
                }
            } else if (ln == "elevation") {
                std::string ev = child.text().get();
                if (!ev.empty() && ev != "NaN") {
                    try { meta.elevation = std::stod(ev); } catch (...) {}
                }
            }
        }

        if (hasId && hasName && !seen[meta.fmisid]) {
            seen[meta.fmisid] = true;
            stations.push_back(std::move(meta));
        }
    }
    return stations;
}

// ─── Build lat/lon key ────────────────────────────────────────────────────

static std::pair<int,int> posKey(double lat, double lon) {
    return {static_cast<int>(std::round(lat * 10000)),
            static_cast<int>(std::round(lon * 10000))};
}

// ─── parseObservations ─────────────────────────────────────────────────────

ParsedObservations WfsParser::parseObservations(const std::string& xml) {
    pugi::xml_document doc;
    auto parseResult = doc.load_string(xml.c_str());
    if (!parseResult) return {};

    auto stations   = extractStations(doc);
    auto fieldNames = extractFieldNames(doc);
    auto positions  = extractPositions(doc);
    auto values     = extractValues(doc);

    LOG_INFO << "parseObservations: fields=" << fieldNames.size()
             << " positions=" << positions.size()
             << " values=" << values.size()
             << " stations=" << stations.size();

    if (fieldNames.empty() || positions.empty() || values.empty())
        return {stations, {}};

    // Map (lat, lon) → station
    std::map<std::pair<int,int>, const services::StationMeta*> latLonMap;
    for (const auto& s : stations)
        latLonMap[posKey(s.latitude, s.longitude)] = &s;

    // Map field names to DB columns
    const int N = static_cast<int>(fieldNames.size());
    std::vector<std::string> dbFields(N);
    for (int i = 0; i < N; ++i) {
        auto it = fmi::OBS_PARAM_MAP.find(fieldNames[i]);
        dbFields[i] = (it != fmi::OBS_PARAM_MAP.end()) ? it->second : "";
    }

    std::vector<Observation> observations;
    for (size_t i = 0; i < positions.size(); ++i) {
        if (static_cast<size_t>(i * N + N) > values.size()) break;

        const auto& pos = positions[i];
        auto it = latLonMap.find(posKey(pos.lat, pos.lon));
        if (it == latLonMap.end()) continue;
        const auto* station = it->second;

        Observation obs;
        obs.fmisid = station->fmisid;
        obs.time   = epochToIso(pos.epoch);

        for (int j = 0; j < N; ++j) {
            const auto& col = dbFields[j];
            const auto& val = values[i * N + j];
            if (col.empty() || !val.has_value()) continue;
            double dv = *val;

            if      (col == "temperature")     obs.temperature      = dv;
            else if (col == "dew_point")        obs.dew_point        = dv;
            else if (col == "humidity")         obs.humidity         = dv;
            else if (col == "wind_speed")       obs.wind_speed       = dv;
            else if (col == "wind_gust")        obs.wind_gust        = dv;
            else if (col == "wind_direction")   obs.wind_direction   = static_cast<int>(std::round(dv));
            else if (col == "precipitation_1h") obs.precipitation_1h = dv;
            else if (col == "precip_intensity") obs.precip_intensity = dv;
            else if (col == "snow_depth")       obs.snow_depth       = dv;
            else if (col == "pressure")         obs.pressure         = dv;
            else if (col == "visibility")       obs.visibility       = static_cast<int>(std::round(dv));
            else if (col == "cloud_cover")      obs.cloud_cover      = static_cast<int>(std::round(dv));
            else if (col == "weather_code")     obs.weather_code     = static_cast<int>(std::round(dv));
        }
        observations.push_back(std::move(obs));
    }

    return {std::move(stations), std::move(observations)};
}

// ─── parseForecasts ────────────────────────────────────────────────────────

ParsedForecasts WfsParser::parseForecasts(const std::string& xml,
                                           const std::string& place,
                                           const std::string& fetchedAt)
{
    pugi::xml_document doc;
    auto parseResult = doc.load_string(xml.c_str());
    if (!parseResult) return {};

    auto fieldNames = extractFieldNames(doc);
    auto positions  = extractPositions(doc);
    auto values     = extractValues(doc);

    // Station metadata (first Location found)
    std::optional<services::StationMeta> stationMeta;
    auto stations = extractStations(doc);
    if (!stations.empty()) stationMeta = stations[0];

    LOG_INFO << "parseForecasts[" << place << "]: fields=" << fieldNames.size()
             << " positions=" << positions.size()
             << " values=" << values.size();

    if (fieldNames.empty() || positions.empty() || values.empty())
        return {stationMeta, {}};

    const int N = static_cast<int>(fieldNames.size());
    std::vector<std::string> dbFields(N);
    for (int i = 0; i < N; ++i) {
        auto it = fmi::FORECAST_PARAM_MAP.find(fieldNames[i]);
        dbFields[i] = (it != fmi::FORECAST_PARAM_MAP.end()) ? it->second : "";
    }

    std::vector<Forecast> forecasts;
    for (size_t i = 0; i < positions.size(); ++i) {
        if (static_cast<size_t>(i * N + N) > values.size()) break;

        const auto& pos = positions[i];

        Forecast fc;
        fc.fetched_at = fetchedAt;
        fc.valid_time = epochToIso(pos.epoch);
        fc.place_name = place;
        fc.model      = "harmonie";
        fc.latitude   = pos.lat;
        fc.longitude  = pos.lon;
        if (stationMeta) fc.fmisid = stationMeta->fmisid;

        for (int j = 0; j < N; ++j) {
            const auto& col = dbFields[j];
            const auto& val = values[i * N + j];
            if (col.empty() || !val.has_value()) continue;
            double dv = *val;

            if      (col == "temperature")     fc.temperature      = dv;
            else if (col == "wind_speed")       fc.wind_speed       = dv;
            else if (col == "wind_direction")   fc.wind_direction   = static_cast<int>(std::round(dv));
            else if (col == "wind_gust")        fc.wind_gust        = dv;
            else if (col == "precipitation_1h") fc.precipitation_1h = dv;
            else if (col == "humidity")         fc.humidity         = dv;
            else if (col == "pressure")         fc.pressure         = dv;
            else if (col == "cloud_cover")      fc.cloud_cover      = dv;
            else if (col == "dew_point")        fc.dew_point        = dv;
            else if (col == "weather_symbol")   fc.weather_symbol   = static_cast<int>(std::round(dv));
        }
        forecasts.push_back(std::move(fc));
    }

    return {stationMeta, std::move(forecasts)};
}

} // namespace weather
