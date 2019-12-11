#pragma once
#include "stm32f4xx.h"
#include <FreeRTOS.h>
#include <task.h>

#define PRINT_CONSOLE_BUFFER_SIZE 400
#define CONSOLE_UART USART3
#define Error_LED_Pin LL_GPIO_PIN_1
#define Error_LED_Port GPIOA
#define DEFAULT_TIMEOUT (1 * configTICK_RATE_HZ)

//=============================================================

using u64 = uint64_t;
using u32 = uint32_t;
using u16 = uint16_t;
using u8 = uint8_t;
using i64 = int64_t;
using i32 = int32_t;
using i16 = int16_t;
using i8 = int8_t;
using f32 = float;
using f64 = double;

inline void enable_cycle_counter(void) {
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    // DWT->LAR = 0xC5ACCE55;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

inline u32 get_cycle_counter_value(void) { return DWT->CYCCNT; }

inline const u32 get_cpu_frequency(void) {
    extern u32 SystemCoreClock;
    return SystemCoreClock;
}

void delay_micros(u32 delay_us);

inline bool isInISR(void) { return (SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) != 0; }

inline u32 getSysTickCount() { return isInISR() ? xTaskGetTickCountFromISR() : xTaskGetTickCount(); }
inline u32 getDeadline(const u32 timeout = DEFAULT_TIMEOUT) { return timeout + getSysTickCount(); }
inline u32 getTimeout(const u32 deadline) {
    i32 ret = deadline - getSysTickCount();
    if (ret < 0) ret = 0;
    return ret;
}

inline void debug_break(void) { __asm__ __volatile__ ("bkpt #0"); }

extern "C"
void FailAndInfiniteLoop();