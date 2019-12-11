#include <xHAL/ThreadUtils>

namespace xHAL {

void waitForNotification(const TaskNotificationIds expectedNotifyId, u32 deadline) {
    u32 notifiedValue = INVALID_NOTIFY_VALUE;
    while (notifiedValue != expectedNotifyId) {
        if (!xTaskNotifyWait(0, 0, &notifiedValue, getTimeout(deadline)))
            FailAndInfiniteLoop();
    }
}

void notifyThread(const TaskHandle_t &caller, const TaskNotificationIds notifyId) {
    BaseType_t shouldYield;
    if (isInISR()) {
        xTaskNotifyFromISR(caller, notifyId, eSetValueWithOverwrite, &shouldYield);
        portYIELD_FROM_ISR(shouldYield);
    } else {
        xTaskNotify(caller, notifyId, eSetValueWithOverwrite);
    }
}

}