#include <drogon/drogon.h>

#include "config/Config.h"
#include "db/ConnectionPool.h"
#include "scheduler/Scheduler.h"

// Include all controllers so their static initialisers run and routes register
#include "controllers/HealthController.h"
#include "controllers/StationsController.h"
#include "controllers/ObservationsController.h"
#include "controllers/ForecastsController.h"
#include "controllers/JobsController.h"

int main() {
    using namespace drogon;

    // ── Initialise config & connection pool ───────────────────────────────
    auto config = weather::Config::fromEnv();
    weather::initConfig(config);
    weather::initPool(config.connString(), 8);

    // ── Initialise scheduler singleton ────────────────────────────────────
    weather::scheduler::initScheduler();

    // ── CORS: add headers to every response ───────────────────────────────
    app().registerPostHandlingAdvice(
        [](const HttpRequestPtr& req, const HttpResponsePtr& resp) {
            resp->addHeader("Access-Control-Allow-Origin",  "*");
            resp->addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
            resp->addHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
        }
    );

    // ── Handle OPTIONS preflight ───────────────────────────────────────────
    // Register a wildcard OPTIONS handler using the internal path regex form.
    // Drogon matches from most-specific to least-specific, so this acts as a
    // catch-all for any /api/v1/... OPTIONS request.
    app().registerHandler("/api/v1/{path:.*}",
        [](const HttpRequestPtr& req,
           std::function<void(const HttpResponsePtr&)>&& cb,
           const std::string& /*path*/) {
            auto resp = HttpResponse::newHttpResponse();
            resp->setStatusCode(k200OK);
            cb(resp);
        },
        {Options}
    );

    // ── Start scheduler threads ───────────────────────────────────────────
    weather::scheduler::getScheduler().start();

    LOG_INFO << "Weather C++ backend starting on port " << config.server_port;

    // ── Start Drogon HTTP server ──────────────────────────────────────────
    app().addListener("0.0.0.0", config.server_port)
         .setThreadNum(4)
         .run();

    // ── Graceful shutdown ─────────────────────────────────────────────────
    weather::scheduler::getScheduler().stop();
    return 0;
}
