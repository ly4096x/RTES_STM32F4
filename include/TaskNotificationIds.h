#pragma once
#include <stdint.h>

enum TaskNotificationId : uint32_t {
    NOTIFY_INVALID_VALUE = 0,
    NOTIFY_USART_CONSOLE_TX_DMA_TC = 5,
    NOTIFY_USART_CONSOLE_TXE,
    NOTIFY_USART_CONSOLE_RXNE,
    NOTIFY_I2C1,

};