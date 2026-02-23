#include "StationsController.h"

#include "../db/ConnectionPool.h"
#include "../services/StationService.h"
#include <drogon/HttpResponse.h>
#include <json/json.h>

namespace weather {

void StationsController::getAll(const drogon::HttpRequestPtr& req,
                                 std::function<void(const drogon::HttpResponsePtr&)>&& callback)
{
    bool activeOnly = true;
    auto activeParam = req->getParameter("active");
    if (activeParam == "false") activeOnly = false;

    try {
        auto stations = services::getStations(getPool(), activeOnly);
        Json::Value arr(Json::arrayValue);
        for (const auto& s : stations) arr.append(s.toJson());
        callback(drogon::HttpResponse::newHttpJsonResponse(arr));
    } catch (const std::exception& e) {
        Json::Value err;
        err["detail"] = e.what();
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k500InternalServerError);
        callback(resp);
    }
}

void StationsController::getOne(const drogon::HttpRequestPtr& req,
                                 std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                                 int fmisid)
{
    try {
        auto station = services::getStation(getPool(), fmisid);
        if (!station) {
            Json::Value err;
            err["detail"] = "Station not found";
            auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
            resp->setStatusCode(drogon::k404NotFound);
            callback(resp);
            return;
        }
        callback(drogon::HttpResponse::newHttpJsonResponse(station->toJson()));
    } catch (const std::exception& e) {
        Json::Value err;
        err["detail"] = e.what();
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k500InternalServerError);
        callback(resp);
    }
}

void StationsController::getLatest(const drogon::HttpRequestPtr& req,
                                    std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                                    int fmisid)
{
    try {
        auto j = services::getStationWithLatest(getPool(), fmisid);
        if (j.isNull()) {
            Json::Value err;
            err["detail"] = "Station not found";
            auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
            resp->setStatusCode(drogon::k404NotFound);
            callback(resp);
            return;
        }
        callback(drogon::HttpResponse::newHttpJsonResponse(j));
    } catch (const std::exception& e) {
        Json::Value err;
        err["detail"] = e.what();
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k500InternalServerError);
        callback(resp);
    }
}

} // namespace weather
