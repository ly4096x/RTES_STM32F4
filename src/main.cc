extern "C" {
#include <FreeRTOS.h>
#include "task.h"
#include <semphr.h>
#include "main.h"
#include "gpio.h"
#include "i2c.h"
#include "tim.h"
#include "dma.h"
#include "usart.h"
#include <cinttypes>
#include <cstdlib>
}
#include "board_common.h"
#include <xHAL/USART>
#include <xHAL/DMA>
#include <xHAL/Shell>
#include "TaskNotificationIds.h"

#include <Adafruit-PWM-Servo-Driver-Library/Adafruit_PWMServoDriver.h>

void idle_thread(void *param);
void main_thread(void *param);
void servo_thread(void *param);

xHAL::DMA console_uart_tx_dma(DMA2, LL_DMA_STREAM_7, NOTIFY_USART_CONSOLE_TX_DMA_TC);
xHAL::USART console(USART1, &console_uart_tx_dma, NOTIFY_USART_CONSOLE_TXE, NOTIFY_USART_CONSOLE_RXNE);
xHAL::ShellCommand cmds[] = {
    {"hello", 0, [](char *)->int { console.printf("hello this is shell\n"); return 0; } },
    {"rx_overflow", 0, [](char *)->int { console.printf("error: rx_overflow\n"); return 1; } },
    {nullptr, 0, nullptr}
};
xHAL::Shell shell(cmds);

extern "C"
int main(void) {
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);

    NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
    NVIC_SetPriority(PendSV_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 15, 0));
    NVIC_SetPriority(SysTick_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 15, 0));

    SystemClock_Config();
    extern uint32_t SystemCoreClock;
    LL_InitTick(SystemCoreClock, 100);


    MX_DMA_Init();
    MX_GPIO_Init();
    MX_I2C1_Init();
    MX_TIM4_Init();
    MX_USART1_UART_Init();
    LL_DMA_SetPeriphAddress(DMA2, LL_DMA_STREAM_7, LL_USART_DMA_GetRegAddr(USART1));

    enable_cycle_counter();

    LL_USART_EnableIT_RXNE(USART1);

    //xTaskCreate(idle_thread, "idle", 256, nullptr, 0, nullptr);
    xTaskCreate(main_thread, "main", 256, nullptr, 10, nullptr);
    xTaskCreate(servo_thread, "servo", 256, nullptr, 30, nullptr);
    xTaskCreate([](void*) { shell.run(); }, "shell", 256, nullptr, 50, nullptr);

    vTaskStartScheduler();
    while (1);
}

void main_thread(void *param) {
    auto thisTask = xTaskGetCurrentTaskHandle();
    uint32_t c = 0;
    while (1) {
        LL_GPIO_TogglePin(LED_OnBoard_GPIO_Port, LED_OnBoard_Pin);
        console.printf("[%s] c = %-10" PRIu32 "  CYCCNT = %10" PRIu32 " STACK_UNUSED = %10" PRIu32 "\n",
            pcTaskGetName(thisTask), c++, get_cycle_counter_value(), uxTaskGetStackHighWaterMark(thisTask));
        delay_micros(1000000);
    }
}

void servo_thread(void *param) {
    auto thisTask = xTaskGetCurrentTaskHandle();
    /*Adafruit_PWMServoDriver pwm;
    pwm.begin();
    pwm.setOscillatorFrequency(25000000);
    pwm.setPWMFreq(400);
    u16 value = 1000;
    while (1) {
        pwm.writeMicroseconds(0, value);
        value += 100;
        if (value > 2000) value = 1000;
        printf("[%s] STACK_UNUSED = %3" PRIu32 " value = %" PRIu16 "\n", pcTaskGetName(thisTask), uxTaskGetStackHighWaterMark(thisTask), value);
        vTaskDelay(10);
    }*/

    u16 val = 0;
    LL_TIM_SetAutoReload(TIM4, 2600);
    LL_TIM_SetPrescaler(TIM4, get_cpu_frequency() / 1000000 - 1);
    LL_TIM_OC_SetCompareCH1(TIM4, val);
    LL_TIM_CC_EnableChannel(TIM4, LL_TIM_CHANNEL_CH1);
    LL_TIM_EnableCounter(TIM4);
    while (1) {
        val += 5;
        if (val > LL_TIM_GetAutoReload(TIM4)) val = 0;
        console.printf("[%s] STACK_UNUSED = %3" PRIu32 " val = %" PRIu16 "\n", pcTaskGetName(thisTask), uxTaskGetStackHighWaterMark(thisTask), val);
        LL_TIM_OC_SetCompareCH1(TIM4, val);
        vTaskDelay(1);
    }
}

void idle_thread(void *param) {
    vTaskDelete(nullptr);
}

void Error_Handler(void) {
    console.printf("ST HAL Error_handler\n");
    FailAndInfiniteLoop();
}