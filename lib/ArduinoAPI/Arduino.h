#pragma once

#include <board_common.h>

inline u32 millis() { return get_cycle_counter_value() / (get_cpu_frequency() / 1000u); }
inline u32 micros() { return get_cycle_counter_value() / (get_cpu_frequency() / 1000000u); }
inline void delayMicroseconds(u32 timeout) { delay_micros(timeout); }
inline void delay(u32 timeout) { delayMicroseconds(timeout * 1000u); }