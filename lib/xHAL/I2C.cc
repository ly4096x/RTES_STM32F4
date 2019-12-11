#include <xHAL/I2C>
#include <xHAL/ThreadUtils>

namespace xHAL {

I2C::I2C(I2C_TypeDef *dev, const TaskNotificationIds _notifyId) :
        dev(dev),
        notifyId(_notifyId)
{}

bool I2C::waitForFlag_SB(u32 deadline) {
    caller = xTaskGetCurrentTaskHandle();
    LL_I2C_EnableIT_EVT(dev);
    waitForNotification(notifyId, deadline);
    LL_I2C_DisableIT_EVT(dev);
}

u32 I2C::write(u8 slaveAddr, bool isRead, u8 *data, u32 len, u32 deadline) {
    while (LL_I2C_IsActiveFlag_BUSY(dev)) ;// TODO deadline expire
    LL_I2C_GenerateStartCondition(dev);
    waitForFlag_SB(deadline);
}

u32 I2C::read(u8 slaveAddr, u8 *data, u32 len, u32 deadline) {

}

void I2C::interruptHandler(const u8 type) {

}

}