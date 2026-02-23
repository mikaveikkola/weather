#pragma once

#include "../db/ConnectionPool.h"
#include "../models/FetchLog.h"

#include <json/json.h>
#include <optional>
#include <string>
#include <vector>

namespace weather::services {

// Insert a new log row; returns the generated id
long long insertFetchLog(ConnectionPool& pool,
                         const std::string& jobType,
                         const std::string& queryParamsJson);

void updateFetchLog(ConnectionPool& pool,
                    long long id,
                    const std::string& status,
                    int recordsFetched,
                    const std::optional<std::string>& errorMsg);

std::vector<FetchLog> getFetchLogs(ConnectionPool& pool,
                                   const std::string& jobTypeFilter,
                                   int limit);

// Returns one entry per job_type (latest)
Json::Value getJobsStatus(ConnectionPool& pool);

} // namespace weather::services
