#pragma once

#include <chrono>
#include <condition_variable>
#include <functional>
#include <map>
#include <mutex>
#include <string>
#include <thread>

namespace weather::scheduler {

class Scheduler {
public:
    Scheduler() = default;
    ~Scheduler();

    void start();
    void stop();

    // Enqueue an immediate run of a named job (from trigger endpoint)
    void triggerNow(const std::string& jobName);

    // Returns ISO 8601 of next scheduled run for a job type ("" if unknown)
    std::string nextRunIso(const std::string& jobType) const;

private:
    void obsLoop();
    void fcLoop();

    std::thread obsThread_;
    std::thread fcThread_;

    mutable std::mutex mutex_;
    std::condition_variable cv_;
    bool stopFlag_{false};

    // Flags to force an immediate run
    bool triggerObs_{false};
    bool triggerFc_{false};

    // Track next-run wall-clock times
    std::chrono::system_clock::time_point nextObsRun_;
    std::chrono::system_clock::time_point nextFcRun_;
};

// Module-level singleton
void       initScheduler();
Scheduler& getScheduler();

} // namespace weather::scheduler
