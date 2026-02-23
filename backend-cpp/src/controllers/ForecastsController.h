#pragma once

#include <drogon/HttpController.h>

namespace weather {

class ForecastsController : public drogon::HttpController<ForecastsController> {
public:
    METHOD_LIST_BEGIN
    // /history must be registered before the base path to avoid mismatching
    ADD_METHOD_TO(ForecastsController::getForecastsHistory, "/api/v1/forecasts/history", drogon::Get);
    ADD_METHOD_TO(ForecastsController::getForecasts,        "/api/v1/forecasts",         drogon::Get);
    METHOD_LIST_END

    // GET /api/v1/forecasts?fmisid=X&place=Helsinki&model=harmonie&hours=48
    void getForecasts(const drogon::HttpRequestPtr& req,
                      std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    // GET /api/v1/forecasts/history?place=Helsinki&model=harmonie&hours=48
    void getForecastsHistory(const drogon::HttpRequestPtr& req,
                             std::function<void(const drogon::HttpResponsePtr&)>&& callback);
};

} // namespace weather
