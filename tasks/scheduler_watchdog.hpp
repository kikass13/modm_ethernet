#pragma once

#include <modm/board.hpp>
#include <modm/io/iostream.hpp>
#include <modm/processing/rtos.hpp>

namespace SchedulerUtils {

class SchedulerInfo {
public:
    SchedulerInfo() { update(); }
    void update() { vTaskList(buf_); }
    const char* toString() const { return const_cast<const char*>(buf_); }

private:
    /*void fillBuffer_(const char* content){
            std::memset(buf_, 0, sizeof(buf_));
            std::strcpy(buf_, content);
            buf_[sizeof(buf_)-1] = '\0'; //terminate for safety
    }*/
public:
private:
    char buf_[2000];
};

class SchedulerInfoGathererTask : modm::rtos::Thread {
public:
    SchedulerInfoGathererTask()
        : Thread(11, 1 << 10, "SchedulerInfoGatherer")
        , schedulerInfo_ {} { }
    void run() {
        MODM_LOG_DEBUG << "  -- Scheduler watchdog gatherer run()" << modm::endl;
        while (true) {
            sleep(500 * MILLISECONDS);
            // see https://www.freertos.org/a00021.html
            // gather relevant information and put them into object
            schedulerInfo_.update();
        }
    }
    const SchedulerInfo& getLatestSchedulerInfo() const { return schedulerInfo_; }

private:
public:
private:
    SchedulerInfo schedulerInfo_;
};

modm::IOStream& operator<<(modm::IOStream& out, const SchedulerInfo& si) {
    out << si.toString();
    return out;
}

class SchedulerWatchdogTask : modm::rtos::Thread {
public:
    SchedulerWatchdogTask()
        : Thread(10, 1 << 10, "SchedulerWatchdog")
        , gathererTask_ {} { }
    void run() {
        MODM_LOG_DEBUG << "  -- Scheduler watchdog task run()" << modm::endl;
        while (true) {
            sleep(5000 * MILLISECONDS);
            MODM_LOG_INFO << "#########################" << modm::endl
                          << modm::endl
                          << gathererTask_.getLatestSchedulerInfo() << "#########################" << modm::endl;
        }
    }

private:
public:
private:
    SchedulerInfoGathererTask gathererTask_;
};

} // namespace SchedulerUtils
