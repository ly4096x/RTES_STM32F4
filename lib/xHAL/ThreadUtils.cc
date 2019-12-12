#include <xHAL/ThreadUtils>

namespace xHAL {

void waitForNotification(const TaskNotificationId expectedNotifyId, u32 deadline, bool waitForever) {
#if 1
    u32 notifiedValue = NOTIFY_INVALID_VALUE;
    while (notifiedValue != expectedNotifyId) {
        if (!xTaskNotifyWait(0, 0, &notifiedValue, waitForever ? portMAX_DELAY : getTimeout(deadline)))
            FailAndInfiniteLoop(); // TODO should return false if timeout
    }
#else // disabled since this workaround (setting caller to NULL after notifying) seems pretty good.
    // This is trying to solve one task being notified multiple times and all previous unmatching notifications got lost
    // only partly solved since the following implementation allow only one unmatching notification to be forwarded
    u32 notifiedValue = NOTIFY_INVALID_VALUE;
    u32 pending = NOTIFY_INVALID_VALUE;
    while (notifiedValue != expectedNotifyId) {
        if (!xTaskNotifyWait(0, 0, &notifiedValue, waitForever ? portMAX_DELAY : getTimeout(deadline)))
            FailAndInfiniteLoop();
        if (notifiedValue != expectedNotifyId) {
            debug_break();
            pending = notifiedValue;
        }
    }
    auto caller = xTaskGetCurrentTaskHandle();
    if (pending != NOTIFY_INVALID_VALUE)
        notifyThread(caller, (TaskNotificationIds)pending);
#endif
}

void notifyThread(TaskHandle_t &caller, const TaskNotificationId notifyId) {
    if (!caller) return;
    if (isInISR()) {
        BaseType_t shouldYield;
        xTaskNotifyFromISR(caller, notifyId, eSetValueWithOverwrite, &shouldYield);
        portYIELD_FROM_ISR(shouldYield);
    } else {
        xTaskNotify(caller, notifyId, eSetValueWithOverwrite);
    }
    caller = nullptr;
}

}