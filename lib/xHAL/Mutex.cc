#include <xHAL/Mutex>

namespace xHAL {

Mutex::Mutex(bool recursive) : recursive(recursive) {
    mutex = recursive ? xSemaphoreCreateRecursiveMutex() : xSemaphoreCreateMutex();
}

bool Mutex::lock(const u32 deadline) {
    bool ret, shouldYield;
    if (isInISR()) {
        ret = xSemaphoreTakeFromISR(mutex, (BaseType_t *)&shouldYield);
        portYIELD_FROM_ISR(shouldYield);
        holder = nullptr;
    } else {
        u32 timeout = getTimeout(deadline);
        ret = recursive ? xSemaphoreTakeRecursive(mutex, timeout) : xSemaphoreTake(mutex, timeout);
        holder = xTaskGetCurrentTaskHandle();
    }
    return ret;
}

bool Mutex::unlock() {
    bool ret, shouldYield;
    if (isInISR()) {
        ret = xSemaphoreGiveFromISR(mutex, (BaseType_t *)&shouldYield);
        portYIELD_FROM_ISR(shouldYield);
    } else {
        ret = recursive ? xSemaphoreGiveRecursive(mutex) : xSemaphoreGive(mutex);
    }
    holder = nullptr;
    return ret;
}

}