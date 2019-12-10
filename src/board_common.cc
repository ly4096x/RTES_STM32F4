extern "C"{
#include "stm32f4xx_ll_gpio.h"
}
#include "board_common.h"

void delay_micros(u32 delay_us) {
    u32 current_CYCCNT = DWT->CYCCNT;
    delay_us *= get_cpu_frequency() / 1000000;
    while (DWT->CYCCNT - current_CYCCNT < delay_us);
}

void FailAndInfiniteLoop(){
    LL_GPIO_ResetOutputPin(Error_LED_Port, Error_LED_Pin);
    while (1) debug_break();
}

extern "C" void __cxa_pure_virtual() { FailAndInfiniteLoop(); }