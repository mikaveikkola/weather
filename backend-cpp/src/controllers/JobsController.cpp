#include "JobsController.h"

#include "../db/ConnectionPool.h"
#include "../services/FetchLogService.h"
#include "../scheduler/Scheduler.h"
#include <drogon/HttpResponse.h>
#include <json/json.h>

namespace weather {

void JobsController::getStatus(const drogon::HttpRequestPtr& req,
                                std::function<void(const drogon::HttpResponsePtr&)>&& callback)
{
    try {
        auto status = services::getJobsStatus(getPool());

        // Enrich with next_run times from the scheduler
        auto& sched = scheduler::getScheduler();
        for (auto& entry : status) {
            std::string jt = entry["job_type"].asString();
            auto nextRun = sched.nextRunIso(jt);
            entry["next_run"] = nextRun.empty() ? Json::Value() : Json::Value(nextRun);
        }

        callback(drogon::HttpResponse::newHttpJsonResponse(status));
    } catch (const std::exception& e) {
        Json::Value err; err["detail"] = e.what();
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k500InternalServerError);
        callback(resp);
    }
}

void JobsController::triggerJob(const drogon::HttpRequestPtr& req,
                                 std::function<void(const drogon::HttpResponsePtr&)>&& callback)
{
    auto body = req->getJsonObject();
    if (!body) {
        Json::Value err; err["detail"] = "JSON body required";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp); return;
    }

    std::string jobName = (*body)["job"].asString();
    if (jobName != "observations" && jobName != "forecast_harmonie") {
        Json::Value err;
        err["detail"] = "Unknown job: " + jobName;
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp); return;
    }

    scheduler::getScheduler().triggerNow(jobName);

    Json::Value j;
    j["status"] = "triggered";
    j["job"]    = jobName;
    callback(drogon::HttpResponse::newHttpJsonResponse(j));
}

void JobsController::getLog(const drogon::HttpRequestPtr& req,
                             std::function<void(const drogon::HttpResponsePtr&)>&& callback)
{
    std::string jobType = req->getParameter("job_type");
    int limit = 50;
    auto limitStr = req->getParameter("limit");
    if (!limitStr.empty()) {
        try { limit = std::stoi(limitStr); } catch (...) {}
        if (limit < 1)   limit = 1;
        if (limit > 500) limit = 500;
    }

    try {
        auto logs = services::getFetchLogs(getPool(), jobType, limit);
        Json::Value arr(Json::arrayValue);
        for (const auto& l : logs) arr.append(l.toJson());
        callback(drogon::HttpResponse::newHttpJsonResponse(arr));
    } catch (const std::exception& e) {
        Json::Value err; err["detail"] = e.what();
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k500InternalServerError);
        callback(resp);
    }
}

} // namespace weather
