#include "xHAL/USART"
#include <xHAL/ThreadUtils>

namespace xHAL {

USART::USART(USART_TypeDef *dev, DMA *dmaTx, const TaskNotificationIds _txNotifyId, const TaskNotificationIds _rxNotifyId) :
        SerialDevice(_txNotifyId, _rxNotifyId),
        dev(dev),
        dmaTx(dmaTx)
{
    if (rxNotifyId != INVALID_NOTIFY_VALUE)
        LL_USART_EnableIT_RXNE(dev);
}

u32 USART::writeWithPolling(u8 *data, u32 len, u32 deadline) {
    if (len == 0) return 0;
    AutoLock lock(txMutex, deadline);
    if (!lock.successful()) return 0;
    
    for (const auto e = data + len; data != e; ++data) {
        while (!LL_USART_IsActiveFlag_TXE(dev))
            if ((i32)deadline - (i32)getSysTickCount() < 0) FailAndInfiniteLoop();
        LL_USART_TransmitData8(dev, *data & 0xFF);
    }
    return len;
}
u32 USART::writeWithInterrupt(u8 *data, u32 len, u32 deadline) {
    if (len == 0) return 0;
    AutoLock lock(txMutex, deadline);
    if (!lock.successful()) return 0;

    tx_data_begin = data;
    tx_data_end = data + len;

    txCaller = xTaskGetCurrentTaskHandle();
    LL_USART_EnableIT_TXE(dev);
    waitForNotification(txNotifyId, deadline);
    return len;
}
u32 USART::writeWithDma(u8 *data, u32 len, u32 deadline) {
    if (len == 0) return 0;
    AutoLock lock(txMutex, deadline);
    if (!lock.successful()) return 0;

    dmaTx->startTransmit(data, len);
    LL_USART_EnableDMAReq_TX(dev);
    dmaTx->waitForComplete(deadline);
    LL_USART_DisableDMAReq_TX(dev);
    return len;
}


void USART::txInterruptHandler() {
    if (tx_data_begin != tx_data_end) {
        LL_USART_TransmitData8(dev, *(tx_data_begin++) & 0xFF);
        return;
    }
    notifyThread(txCaller, txNotifyId);
    LL_USART_DisableIT_TXE(dev);
}

void USART::rxInterruptHandler() {
    *rxbuf_end = LL_USART_ReceiveData8(dev);
    bool overflow = 0;
    if (rxbuf_end + 1 == rxbuf_begin || rxbuf_end + 1 - sizeof(rxbuf) == rxbuf_begin)
        overflow = 1;
    if (*rxbuf_end == '\r') return;
    if (*rxbuf_end == '\n' || *rxbuf_end == 0x03 || overflow || rxEcho) { // newline or ctrl-c or overflow
        notifyThread(rxCaller, rxNotifyId);
    }
    if (overflow) return;
    ++rxbuf_end;
    if (rxbuf_end == rxbuf + sizeof(rxbuf)) rxbuf_end = rxbuf;
}

} // namespace xHAL
