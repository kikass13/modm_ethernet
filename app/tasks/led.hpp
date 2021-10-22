#pragma once

#include <modm/board.hpp>
#include <modm/processing/rtos.hpp>

template <typename GPIO_T> class LedTask : modm::rtos::Thread {
public:
    LedTask()
        : Thread(configMAX_PRIORITIES-1, 1 << 10) { }
    void run() {
        MODM_LOG_DEBUG << "  -- Led blink task run()" << modm::endl;
        GPIO_T::setOutput();
        while (true) {
            sleep(500 * MILLISECONDS);
            GPIO_T::toggle();
        }
    }
};