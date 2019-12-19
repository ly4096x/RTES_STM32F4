#pragma once

#include <board_common.h>

inline u32 millis() { return get_cycle_counter_value() / (get_cpu_frequency() / 1000u); }
inline u32 micros() { return get_cycle_counter_value() / (get_cpu_frequency() / 1000000u); }
inline void delayMicroseconds(u32 timeout) { delay_micros(timeout); }
inline void delay(u32 timeout) {
    const u32 ms_per_tick = 1000u / configTICK_RATE_HZ;
    vTaskDelay(timeout / ms_per_tick);
    delayMicroseconds((timeout % ms_per_tick) * 1000u);
}