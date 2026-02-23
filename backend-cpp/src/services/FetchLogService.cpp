#include "FetchLogService.h"

#include <pqxx/pqxx>

namespace weather::services {

long long insertFetchLog(ConnectionPool& pool,
                         const std::string& jobType,
                         const std::string& queryParamsJson)
{
    auto conn = pool.acquire();
    pqxx::work txn(conn.get());

    auto rows = txn.exec_params(
        "INSERT INTO fetch_log (job_type, status, query_params) "
        "VALUES ($1, 'running', $2::jsonb) RETURNING id",
        jobType,
        queryParamsJson.empty() ? std::optional<std::string>{} : std::make_optional(queryParamsJson)
    );
    txn.commit();
    return rows[0][0].as<long long>();
}

void updateFetchLog(ConnectionPool& pool,
                    long long id,
                    const std::string& status,
                    int recordsFetched,
                    const std::optional<std::string>& errorMsg)
{
    auto conn = pool.acquire();
    pqxx::work txn(conn.get());

    txn.exec_params(
        "UPDATE fetch_log SET "
        "  status=$2, records_fetched=$3, error_message=$4, finished_at=NOW() "
        "WHERE id=$1",
        id, status, recordsFetched, errorMsg
    );
    txn.commit();
}

std::vector<FetchLog> getFetchLogs(ConnectionPool& pool,
                                   const std::string& jobTypeFilter,
                                   int limit)
{
    auto conn = pool.acquire();
    pqxx::work txn(conn.get());

    pqxx::result rows;
    if (jobTypeFilter.empty()) {
        rows = txn.exec_params(
            "SELECT id, job_type, status, records_fetched, error_message, "
            "  to_char(started_at  AT TIME ZONE 'UTC', 'YYYY-MM-DD\"T\"HH24:MI:SS\"Z\"') AS started_at,"
            "  to_char(finished_at AT TIME ZONE 'UTC', 'YYYY-MM-DD\"T\"HH24:MI:SS\"Z\"') AS finished_at "
            "FROM fetch_log ORDER BY started_at DESC LIMIT $1",
            limit
        );
    } else {
        rows = txn.exec_params(
            "SELECT id, job_type, status, records_fetched, error_message, "
            "  to_char(started_at  AT TIME ZONE 'UTC', 'YYYY-MM-DD\"T\"HH24:MI:SS\"Z\"') AS started_at,"
            "  to_char(finished_at AT TIME ZONE 'UTC', 'YYYY-MM-DD\"T\"HH24:MI:SS\"Z\"') AS finished_at "
            "FROM fetch_log WHERE job_type=$1 ORDER BY started_at DESC LIMIT $2",
            jobTypeFilter, limit
        );
    }

    std::vector<FetchLog> result;
    for (const auto& r : rows) {
        FetchLog log;
        log.id              = r["id"].as<long long>();
        log.job_type        = r["job_type"].as<std::string>();
        log.status          = r["status"].as<std::string>();
        log.records_fetched = r["records_fetched"].as<int>();
        log.started_at      = r["started_at"].is_null()  ? "" : r["started_at"].as<std::string>();
        log.finished_at     = r["finished_at"].is_null() ? std::nullopt
                              : std::make_optional(r["finished_at"].as<std::string>());
        log.error_message   = r["error_message"].is_null() ? std::nullopt
                              : std::make_optional(r["error_message"].as<std::string>());
        result.push_back(std::move(log));
    }
    return result;
}

Json::Value getJobsStatus(ConnectionPool& pool) {
    auto conn = pool.acquire();
    pqxx::work txn(conn.get());

    auto rows = txn.exec(
        "SELECT DISTINCT ON (job_type) "
        "  id, job_type, status, records_fetched, "
        "  to_char(started_at  AT TIME ZONE 'UTC', 'YYYY-MM-DD\"T\"HH24:MI:SS\"Z\"') AS started_at,"
        "  to_char(finished_at AT TIME ZONE 'UTC', 'YYYY-MM-DD\"T\"HH24:MI:SS\"Z\"') AS finished_at "
        "FROM fetch_log "
        "ORDER BY job_type, started_at DESC"
    );

    Json::Value arr(Json::arrayValue);
    for (const auto& r : rows) {
        Json::Value j;
        j["id"]              = static_cast<Json::Int64>(r["id"].as<long long>());
        j["job_type"]        = r["job_type"].as<std::string>();
        j["status"]          = r["status"].as<std::string>();
        j["records_fetched"] = r["records_fetched"].as<int>();
        j["started_at"]      = r["started_at"].is_null()  ? Json::Value() : Json::Value(r["started_at"].as<std::string>());
        j["finished_at"]     = r["finished_at"].is_null() ? Json::Value() : Json::Value(r["finished_at"].as<std::string>());
        j["next_run"]        = Json::Value();  // populated by Scheduler
        arr.append(j);
    }
    return arr;
}

} // namespace weather::services
