#pragma once

#include <drogon/HttpController.h>

namespace weather {

class StationsController : public drogon::HttpController<StationsController> {
public:
    METHOD_LIST_BEGIN
    // /latest must be registered before /{fmisid} to avoid shadowing
    ADD_METHOD_TO(StationsController::getAll,    "/api/v1/stations",               drogon::Get);
    ADD_METHOD_TO(StationsController::getOne,    "/api/v1/stations/{fmisid}",      drogon::Get);
    ADD_METHOD_TO(StationsController::getLatest, "/api/v1/stations/{fmisid}/latest", drogon::Get);
    METHOD_LIST_END

    void getAll(const drogon::HttpRequestPtr& req,
                std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    void getOne(const drogon::HttpRequestPtr& req,
                std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                int fmisid);

    void getLatest(const drogon::HttpRequestPtr& req,
                   std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                   int fmisid);
};

} // namespace weather
