#include "ForecastsController.h"

#include "../db/ConnectionPool.h"
#include "../models/Forecast.h"
#include "../services/ForecastService.h"
#include "../services/StationService.h"
#include <drogon/HttpResponse.h>
#include <json/json.h>

namespace weather {

void ForecastsController::getForecasts(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback)
{
    auto fmisidStr = req->getParameter("fmisid");
    auto place     = req->getParameter("place");
    auto model     = req->getParameter("model");
    if (model.empty()) model = "harmonie";

    if (fmisidStr.empty() && place.empty()) {
        Json::Value err;
        err["detail"] = "Either fmisid or place is required";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp);
        return;
    }

    try {
        std::vector<Forecast> forecasts;
        Json::Value stationJson;

        if (!fmisidStr.empty()) {
            int fmisid = std::stoi(fmisidStr);
            auto station = services::getStation(getPool(), fmisid);
            if (!station) {
                Json::Value err; err["detail"] = "Station not found";
                auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
                resp->setStatusCode(drogon::k404NotFound);
                callback(resp); return;
            }
            stationJson = station->toJson();
            forecasts = services::getForecastsByFmisid(getPool(), fmisid, model);
        } else {
            forecasts = services::getForecastsByPlace(getPool(), place, model);
        }

        Json::Value j;
        j["station"]    = stationJson.isNull() ? Json::Value() : stationJson;
        j["model"]      = model;
        j["fetched_at"] = forecasts.empty() ? Json::Value() : Json::Value(forecasts[0].fetched_at);
        Json::Value arr(Json::arrayValue);
        for (const auto& f : forecasts) arr.append(f.toJson());
        j["data"] = arr;

        callback(drogon::HttpResponse::newHttpJsonResponse(j));
    } catch (const std::exception& e) {
        Json::Value err; err["detail"] = e.what();
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k500InternalServerError);
        callback(resp);
    }
}

void ForecastsController::getForecastsHistory(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback)
{
    auto place = req->getParameter("place");
    auto model = req->getParameter("model");
    if (model.empty()) model = "harmonie";

    int hours = 48;
    auto hoursStr = req->getParameter("hours");
    if (!hoursStr.empty()) {
        try { hours = std::stoi(hoursStr); } catch (...) {}
    }

    if (place.empty()) {
        Json::Value err;
        err["detail"] = "place is required";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp);
        return;
    }

    try {
        auto forecasts = services::getForecastsHistoryByPlace(getPool(), place, model, hours);

        Json::Value j;
        j["station"]    = Json::Value();
        j["model"]      = model;
        j["fetched_at"] = forecasts.empty() ? Json::Value() : Json::Value(forecasts[0].fetched_at);
        Json::Value arr(Json::arrayValue);
        for (const auto& f : forecasts) arr.append(f.toJson());
        j["data"] = arr;

        callback(drogon::HttpResponse::newHttpJsonResponse(j));
    } catch (const std::exception& e) {
        Json::Value err; err["detail"] = e.what();
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k500InternalServerError);
        callback(resp);
    }
}

} // namespace weather
