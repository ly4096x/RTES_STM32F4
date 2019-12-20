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
}
#include <cinttypes>
#include "board_common.h"
#include <xHAL/USART>
#include <xHAL/DMA>
#include <xHAL/I2C>
#include <xHAL/Shell>
#include "TaskNotificationIds.h"

void blink_thread(void *param);
void main_thread(void *param);
void servo_thread(void *param);
extern void vlq_thread(void *param);
extern void motor_thread(void *param);
extern void navigation_thread(void *param);
extern void rssi_receiver_1_thread(void *param);
extern void rssi_receiver_2_thread(void *param);

xHAL::DMA console_uart_tx_dma(DMA2, LL_DMA_STREAM_7, NOTIFY_USART_CONSOLE_TX_DMA_TC);
xHAL::USART console(USART1, &console_uart_tx_dma, NOTIFY_USART_CONSOLE_TXE, NOTIFY_USART_CONSOLE_RXNE);
xHAL::I2C xI2C1(I2C1, NOTIFY_I2C1);
xHAL::USART xUART4(UART4, nullptr, NOTIFY_INVALID_VALUE, NOTIFY_UART4_RXNE);
xHAL::USART xUART5(UART5, nullptr, NOTIFY_INVALID_VALUE, NOTIFY_UART5_RXNE);

extern xHAL::ShellCommand cmds[];
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
    enable_cycle_counter();

    LL_EXTI_DisableIT_0_31(LL_EXTI_LINE_0);
    MX_GPIO_Init();
    LL_EXTI_DisableIT_0_31(LL_EXTI_LINE_0);
    MX_DMA_Init();
    MX_I2C1_Init();
    MX_TIM3_Init();
    MX_TIM4_Init();
    MX_TIM7_Init();
    MX_TIM12_Init();
    MX_TIM14_Init();
    MX_USART1_UART_Init();
    MX_UART4_Init();
    MX_UART5_Init();
    LL_DMA_SetPeriphAddress(DMA2, LL_DMA_STREAM_7, LL_USART_DMA_GetRegAddr(USART1));

    LL_GPIO_ResetOutputPin(GPIOE, LL_GPIO_PIN_1);
    delay_micros(100000);

    u32 deadline = get_cycle_counter_value() + SystemCoreClock / 1000 * 100;
    while (LL_USART_IsActiveFlag_RXNE(USART1) && get_cycle_counter_value() < deadline)
        LL_USART_ReceiveData8(USART1);

    LL_GPIO_ResetOutputPin(ESP1_EN_GPIO_Port, ESP1_EN_Pin);
    LL_GPIO_ResetOutputPin(ESP2_EN_GPIO_Port, ESP2_EN_Pin);

    console.terminal_mode = true;
    LL_USART_EnableIT_RXNE(USART1);
    LL_USART_EnableIT_RXNE(UART4);
    LL_USART_EnableIT_RXNE(UART5);

    extern TaskHandle_t *tasklistend;
    //xTaskCreate(idle_thread, "idle", 256, nullptr, 0, nullptr);
    xTaskCreate(blink_thread, "_blink", 128, nullptr, 10, tasklistend++);
    xTaskCreate(main_thread, "main", 256, nullptr, 10, tasklistend++);
    xTaskCreate(servo_thread, "servo", 256, nullptr, 30, tasklistend++);
    xTaskCreate([](void*) { shell.run(); }, "shell", 256, nullptr, 50, tasklistend++);
    xTaskCreate(vlq_thread, "vlq", 256, nullptr, 20, tasklistend++);
    xTaskCreate(motor_thread, "motor", 256, nullptr, 20, tasklistend++);
    xTaskCreate(navigation_thread, "nav", 256, nullptr, 40, tasklistend++);
    xTaskCreate(rssi_receiver_1_thread, "rssi1", 256, nullptr, 30, tasklistend++);
    xTaskCreate(rssi_receiver_2_thread, "rssi2", 256, nullptr, 30, tasklistend++);

    LL_GPIO_SetOutputPin(GPIOE, LL_GPIO_PIN_1);

    vTaskStartScheduler();
    FailAndInfiniteLoop();
}

void main_thread(void *param) {
    auto thisTask = xTaskGetCurrentTaskHandle();
    vTaskSuspend(nullptr);

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
    
    u16 val = 1500;
    auto dev = TIM14;
    LL_TIM_SetAutoReload(dev, 2600);
    LL_TIM_EnableARRPreload(dev);
    LL_TIM_SetPrescaler(dev, get_cpu_frequency() / 2 / 1000000 - 1);
    LL_TIM_OC_SetCompareCH1(dev, val);
    LL_TIM_CC_EnableChannel(dev, LL_TIM_CHANNEL_CH1);
    LL_TIM_OC_SetCompareCH1(TIM14, 1500);
    LL_TIM_EnableCounter(dev);
    
    
    vTaskSuspend(nullptr);

    while (1) {
        val += 5;
        if (val > LL_TIM_GetAutoReload(dev)) val = 0;
        console.printf("[%s] STACK_UNUSED = %3" PRIu32 " val = %" PRIu16 "\n", pcTaskGetName(thisTask), uxTaskGetStackHighWaterMark(thisTask), val);
        LL_TIM_OC_SetCompareCH1(dev, val);
        vTaskDelay(1);
    }
}

void blink_thread(void *param) {
    extern u32 blink_sequence[4];
    while (true) {
        LL_GPIO_ResetOutputPin(LED_OnBoard_GPIO_Port, LED_OnBoard_Pin);
        vTaskDelay(blink_sequence[0]);
        LL_GPIO_SetOutputPin(LED_OnBoard_GPIO_Port, LED_OnBoard_Pin);
        vTaskDelay(blink_sequence[1]);
        LL_GPIO_ResetOutputPin(LED_OnBoard_GPIO_Port, LED_OnBoard_Pin);
        vTaskDelay(blink_sequence[2]);
        LL_GPIO_SetOutputPin(LED_OnBoard_GPIO_Port, LED_OnBoard_Pin);
        vTaskDelay(blink_sequence[3]);
    }
}

void Error_Handler(void) {
    console.printf("ST HAL Error_handler\n");
    FailAndInfiniteLoop();
}