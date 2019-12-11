#include <xHAL/ThreadUtils>

namespace xHAL {

void waitForNotification(const TaskNotificationIds expectedNotifyId, u32 deadline, bool waitForever) {
    u32 notifiedValue = INVALID_NOTIFY_VALUE;
    while (notifiedValue != expectedNotifyId) {
        if (!xTaskNotifyWait(0, 0, &notifiedValue, waitForever ? portMAX_DELAY : getTimeout(deadline)))
            FailAndInfiniteLoop();
    }
}

void notifyThread(const TaskHandle_t &caller, const TaskNotificationIds notifyId) {
    if (isInISR()) {
        BaseType_t shouldYield;
        xTaskNotifyFromISR(caller, notifyId, eSetValueWithOverwrite, &shouldYield);
        portYIELD_FROM_ISR(shouldYield);
    } else {
        xTaskNotify(caller, notifyId, eSetValueWithOverwrite);
    }
}

}