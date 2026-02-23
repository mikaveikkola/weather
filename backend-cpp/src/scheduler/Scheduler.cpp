#include "Scheduler.h"
#include "Jobs.h"
#include "../config/Config.h"

#include <chrono>
#include <ctime>
#include <memory>
#include <stdexcept>

namespace weather::scheduler {

// ─── Singleton ─────────────────────────────────────────────────────────────

static std::unique_ptr<Scheduler> g_scheduler;

void initScheduler() {
    g_scheduler = std::make_unique<Scheduler>();
}

Scheduler& getScheduler() {
    if (!g_scheduler) throw std::runtime_error("Scheduler not initialised");
    return *g_scheduler;
}

// ─── Destructor ────────────────────────────────────────────────────────────

Scheduler::~Scheduler() {
    stop();
}

// ─── start / stop ──────────────────────────────────────────────────────────

void Scheduler::start() {
    obsThread_ = std::thread(&Scheduler::obsLoop, this);
    fcThread_  = std::thread(&Scheduler::fcLoop,  this);
}

void Scheduler::stop() {
    {
        std::unique_lock<std::mutex> lock(mutex_);
        stopFlag_ = true;
    }
    cv_.notify_all();
    if (obsThread_.joinable()) obsThread_.join();
    if (fcThread_.joinable())  fcThread_.join();
}

void Scheduler::triggerNow(const std::string& jobName) {
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (jobName == "observations")     triggerObs_ = true;
        else if (jobName == "forecast_harmonie") triggerFc_  = true;
    }
    cv_.notify_all();
}

std::string Scheduler::nextRunIso(const std::string& jobType) const {
    std::unique_lock<std::mutex> lock(mutex_);
    std::chrono::system_clock::time_point tp;
    if (jobType == "observations")       tp = nextObsRun_;
    else if (jobType == "forecast_harmonie") tp = nextFcRun_;
    else return "";

    time_t t = std::chrono::system_clock::to_time_t(tp);
    if (t <= 0) return "";
    struct tm utc{};
    gmtime_r(&t, &utc);
    char buf[32];
    strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &utc);
    return buf;
}

// ─── Observation loop ──────────────────────────────────────────────────────

void Scheduler::obsLoop() {
    const int intervalMin = getConfig().fetch_interval_minutes;

    while (true) {
        // Update next-run time
        {
            std::unique_lock<std::mutex> lock(mutex_);
            nextObsRun_ = std::chrono::system_clock::now()
                          + std::chrono::minutes(intervalMin);
        }

        fetchObservations();

        std::unique_lock<std::mutex> lock(mutex_);
        auto deadline = std::chrono::steady_clock::now()
                        + std::chrono::minutes(intervalMin);

        cv_.wait_until(lock, deadline, [this] {
            return stopFlag_ || triggerObs_;
        });

        if (stopFlag_) break;
        triggerObs_ = false;
    }
}

// ─── Forecast loop ─────────────────────────────────────────────────────────

void Scheduler::fcLoop() {
    const int intervalMin = getConfig().forecast_interval_minutes;

    while (true) {
        // Update next-run time
        {
            std::unique_lock<std::mutex> lock(mutex_);
            nextFcRun_ = std::chrono::system_clock::now()
                         + std::chrono::minutes(intervalMin);
        }

        fetchForecasts();

        std::unique_lock<std::mutex> lock(mutex_);
        auto deadline = std::chrono::steady_clock::now()
                        + std::chrono::minutes(intervalMin);

        cv_.wait_until(lock, deadline, [this] {
            return stopFlag_ || triggerFc_;
        });

        if (stopFlag_) break;
        triggerFc_ = false;
    }
}

} // namespace weather::scheduler
