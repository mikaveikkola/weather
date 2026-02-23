#include "ObservationsController.h"

#include "../db/ConnectionPool.h"
#include "../services/ObservationService.h"
#include "../services/StationService.h"
#include <drogon/HttpResponse.h>
#include <json/json.h>
#include <sstream>
#include <vector>

namespace weather {

void ObservationsController::getTimeSeries(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback)
{
    auto fmisidStr = req->getParameter("fmisid");
    if (fmisidStr.empty()) {
        Json::Value err; err["detail"] = "fmisid is required";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp); return;
    }

    int fmisid = 0;
    try { fmisid = std::stoi(fmisidStr); }
    catch (...) {
        Json::Value err; err["detail"] = "Invalid fmisid";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp); return;
    }

    std::string start      = req->getParameter("start");
    std::string end        = req->getParameter("end");
    std::string resolution = req->getParameter("resolution");
    if (resolution.empty()) resolution = "auto";

    try {
        auto station = services::getStation(getPool(), fmisid);
        if (!station) {
            Json::Value err; err["detail"] = "Station not found";
            auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
            resp->setStatusCode(drogon::k404NotFound);
            callback(resp); return;
        }

        auto data = services::getObservations(getPool(), fmisid, start, end, resolution);

        Json::Value j;
        j["station"]    = station->toJson();
        j["resolution"] = resolution;
        j["data"]       = data;
        callback(drogon::HttpResponse::newHttpJsonResponse(j));
    } catch (const std::exception& e) {
        Json::Value err; err["detail"] = e.what();
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k500InternalServerError);
        callback(resp);
    }
}

void ObservationsController::getLatest(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback)
{
    std::vector<int> fmisids;
    auto fmisidsParam = req->getParameter("fmisids");
    if (!fmisidsParam.empty()) {
        std::istringstream iss(fmisidsParam);
        std::string token;
        while (std::getline(iss, token, ',')) {
            try { fmisids.push_back(std::stoi(token)); }
            catch (...) {
                Json::Value err; err["detail"] = "Invalid fmisids parameter";
                auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
                resp->setStatusCode(drogon::k400BadRequest);
                callback(resp); return;
            }
        }
    }

    try {
        auto data = services::getLatestObservations(getPool(), fmisids);
        callback(drogon::HttpResponse::newHttpJsonResponse(data));
    } catch (const std::exception& e) {
        Json::Value err; err["detail"] = e.what();
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k500InternalServerError);
        callback(resp);
    }
}

void ObservationsController::getSummary(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback)
{
    auto fmisidStr = req->getParameter("fmisid");
    if (fmisidStr.empty()) {
        Json::Value err; err["detail"] = "fmisid is required";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp); return;
    }

    int fmisid = 0;
    try { fmisid = std::stoi(fmisidStr); }
    catch (...) {
        Json::Value err; err["detail"] = "Invalid fmisid";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp); return;
    }

    std::string period = req->getParameter("period");
    if (period.empty()) period = "24h";

    try {
        auto station = services::getStation(getPool(), fmisid);
        if (!station) {
            Json::Value err; err["detail"] = "Station not found";
            auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
            resp->setStatusCode(drogon::k404NotFound);
            callback(resp); return;
        }

        auto summary = services::getSummary(getPool(), fmisid, period);
        callback(drogon::HttpResponse::newHttpJsonResponse(summary));
    } catch (const std::exception& e) {
        Json::Value err; err["detail"] = e.what();
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k500InternalServerError);
        callback(resp);
    }
}

} // namespace weather
