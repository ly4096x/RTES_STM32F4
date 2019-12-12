#include <xHAL/I2C>
#include <xHAL/ThreadUtils>
#include <xHAL/USART>

extern xHAL::USART console;

namespace xHAL {

I2C::I2C(I2C_TypeDef *dev, const TaskNotificationId _notifyId) :
        dev(dev),
        notifyId(_notifyId),
        state(STATE_STOPPED)
{}

bool I2C::isError() {
    return LL_I2C_IsActiveFlag_ARLO(dev) || LL_I2C_IsActiveFlag_BERR(dev) || LL_I2C_IsActiveFlag_OVR(dev) || LL_I2C_IsActiveFlag_AF(dev);
}

u16 I2C::startTransaction(u8 slaveAddr, bool isRead, u8 *data, u16 len, bool genStop, u32 deadline) {
    AutoLock lock(mutex, deadline);
    if (!lock.successful()) return 0;

    while (state == STATE_STOPPED && LL_I2C_IsActiveFlag_BUSY(dev)) if (deadline == getSysTickCount()) return 0;

    cmd.addr = slaveAddr;
    cmd.data = data;
    cmd.isRead = isRead;
    cmd.len = len;
    cmd.genStop = genStop;

    caller = xTaskGetCurrentTaskHandle();
    state = STATE_WAITING_START;
    LL_I2C_GenerateStartCondition(dev);
    LL_I2C_EnableIT_EVT(dev);
    waitForNotification(notifyId, deadline);

    LL_I2C_DisableIT_EVT(dev);
    LL_I2C_DisableIT_ERR(dev);
    LL_I2C_DisableIT_BUF(dev);
    if (state == STATE_ERROR) {
        clearInterruptFlags();
        state = STATE_STOPPED;
        return 0;
    }
    return len;
}

void I2C::clearInterruptFlags() {
    LL_I2C_ClearFlag_AF(dev);
    LL_I2C_ClearFlag_OVR(dev);
    LL_I2C_ClearFlag_ARLO(dev);
    LL_I2C_ClearFlag_BERR(dev);
}

void I2C::interruptHandler(bool err) {
    if (err) {
        LL_I2C_DisableIT_EVT(dev);
        LL_I2C_DisableIT_ERR(dev);
        LL_I2C_GenerateStopCondition(dev);
        state = STATE_ERROR;
        notifyThread(caller, notifyId);
        return;
    }

    switch (state) {
    case STATE_WAITING_START:
        console.printf("i2c sr1 = %08x\n", dev->SR1);
        if (LL_I2C_IsActiveFlag_SB(dev)) {
            LL_I2C_TransmitData8(dev, (cmd.addr << 1) | (cmd.isRead & 1));
            cmd.data_ptr = cmd.data;
            state = STATE_WAITING_ADDR;
        }
        return;
    case STATE_WAITING_ADDR:
        if (LL_I2C_IsActiveFlag_ADDR(dev)) {
            LL_I2C_ClearFlag_ADDR(dev);
            if (cmd.len == 0) {
                if (cmd.genStop) LL_I2C_GenerateStopCondition(dev);
                state = cmd.genStop ? STATE_STOPPED : STATE_WAITING_START;
                notifyThread(caller, notifyId);
                return;
            }
            if (cmd.isRead) {
                LL_I2C_AcknowledgeNextData(dev, cmd.len == 1 ? LL_I2C_NACK : LL_I2C_ACK);
                state = STATE_WAITING_RXNE;
            } else {
                LL_I2C_TransmitData8(dev, *cmd.data_ptr++);
                state = STATE_WAITING_TXE;
            }
            LL_I2C_EnableIT_BUF(dev);
        }
        return;
    case STATE_WAITING_TXE:
        if (LL_I2C_IsActiveFlag_TXE(dev)) {
            LL_I2C_TransmitData8(dev, *cmd.data_ptr++);
            if (cmd.data_ptr - cmd.data == cmd.len) {
                if (cmd.genStop) LL_I2C_GenerateStopCondition(dev);
                state = cmd.genStop ? STATE_STOPPED : STATE_WAITING_START;
                notifyThread(caller, notifyId);
            }
        }
        return;
    case STATE_WAITING_RXNE:
        if (LL_I2C_IsActiveFlag_RXNE(dev)) {
            *cmd.data_ptr++ = LL_I2C_ReceiveData8(dev);
            if (cmd.data_ptr + 1 - cmd.data == cmd.len) {
                LL_I2C_AcknowledgeNextData(dev, LL_I2C_NACK);
            } else if (cmd.data_ptr - cmd.data == cmd.len) {
                if (cmd.genStop) LL_I2C_GenerateStopCondition(dev);
                state = cmd.genStop ? STATE_STOPPED : STATE_WAITING_START;
                notifyThread(caller, notifyId);
            }
        }
        return;
    default:
        debug_break();
    }
    FailAndInfiniteLoop();
}

}