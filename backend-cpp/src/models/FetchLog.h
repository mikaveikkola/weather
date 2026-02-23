#pragma once

#include <json/json.h>
#include <optional>
#include <string>

namespace weather {

struct FetchLog {
    long long   id{0};
    std::string started_at;
    std::optional<std::string> finished_at;
    std::string job_type;
    std::string status;
    int         records_fetched{0};
    std::optional<std::string> error_message;
    std::optional<std::string> query_params;  // JSON string for JSONB column

    Json::Value toJson() const {
        Json::Value j;
        j["id"]              = static_cast<Json::Int64>(id);
        j["started_at"]      = started_at;
        j["finished_at"]     = finished_at  ? Json::Value(*finished_at)  : Json::Value();
        j["job_type"]        = job_type;
        j["status"]          = status;
        j["records_fetched"] = records_fetched;
        j["error_message"]   = error_message ? Json::Value(*error_message) : Json::Value();
        return j;
    }
};

} // namespace weather
