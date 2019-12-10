#pragma once
#include <stdint.h>

enum TaskNotificationIds : uint32_t {
    INVALID_NOTIFY_VALUE = 0,
    NOTIFY_USART_CONSOLE_TX_DMA_TC = 5,
    NOTIFY_USART_CONSOLE_TXE,
    NOTIFY_USART_CONSOLE_RXNE
};