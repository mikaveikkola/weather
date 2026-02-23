#pragma once

#include <drogon/HttpController.h>

namespace weather {

class ObservationsController : public drogon::HttpController<ObservationsController> {
public:
    METHOD_LIST_BEGIN
    // Specific sub-paths must come BEFORE parameterised ones
    ADD_METHOD_TO(ObservationsController::getLatest,  "/api/v1/observations/latest",  drogon::Get);
    ADD_METHOD_TO(ObservationsController::getSummary, "/api/v1/observations/summary", drogon::Get);
    ADD_METHOD_TO(ObservationsController::getTimeSeries, "/api/v1/observations",      drogon::Get);
    METHOD_LIST_END

    // GET /api/v1/observations?fmisid=X&start=T&end=T&resolution=auto
    void getTimeSeries(const drogon::HttpRequestPtr& req,
                       std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    // GET /api/v1/observations/latest?fmisids=X,Y
    void getLatest(const drogon::HttpRequestPtr& req,
                   std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    // GET /api/v1/observations/summary?fmisid=X&period=24h
    void getSummary(const drogon::HttpRequestPtr& req,
                    std::function<void(const drogon::HttpResponsePtr&)>&& callback);
};

} // namespace weather
