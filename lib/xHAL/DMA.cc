#include <xHAL/DMA>
#include <xHAL/ThreadUtils>

namespace xHAL {

DMA::DMA(DMA_TypeDef *dev, u32 channel, const TaskNotificationIds _notifyId) :
        dma_dev(dev),
        dma_channel(channel),
        notifyId(_notifyId)
{
    LL_DMA_DisableStream(dma_dev, dma_channel);
    while (LL_DMA_IsEnabledStream(dma_dev, dma_channel));
}

void DMA::startTransmit(u8 *from, u32 len) {
    caller = xTaskGetCurrentTaskHandle();
    LL_DMA_SetMemoryAddress(dma_dev, dma_channel, (u32)from);
    LL_DMA_SetDataLength(dma_dev, dma_channel, len);
    LL_DMA_EnableIT_TC(dma_dev, dma_channel);
    LL_DMA_EnableIT_TE(dma_dev, dma_channel);
    LL_DMA_EnableStream(dma_dev, dma_channel);
}

void DMA::waitForComplete(u32 deadline) {
    caller = xTaskGetCurrentTaskHandle();
    waitForNotification(notifyId, deadline);
}

void DMA::interruptHandler(const bool TC, const bool TE) {
    if (TC) {
        notifyThread(caller, notifyId);
    }
    if (TE) {
        FailAndInfiniteLoop();
    }
    
    LL_DMA_DisableIT_TE(dma_dev, dma_channel);
    LL_DMA_DisableIT_TC(dma_dev, dma_channel);
    LL_DMA_DisableStream(dma_dev, dma_channel);
}

}