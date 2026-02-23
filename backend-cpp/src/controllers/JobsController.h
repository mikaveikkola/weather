#pragma once

#include <drogon/HttpController.h>

namespace weather {

class JobsController : public drogon::HttpController<JobsController> {
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(JobsController::getStatus,  "/api/v1/jobs/status",  drogon::Get);
    ADD_METHOD_TO(JobsController::triggerJob, "/api/v1/jobs/trigger", drogon::Post);
    ADD_METHOD_TO(JobsController::getLog,     "/api/v1/jobs/log",     drogon::Get);
    METHOD_LIST_END

    void getStatus(const drogon::HttpRequestPtr& req,
                   std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    void triggerJob(const drogon::HttpRequestPtr& req,
                    std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    void getLog(const drogon::HttpRequestPtr& req,
                std::function<void(const drogon::HttpResponsePtr&)>&& callback);
};

} // namespace weather
