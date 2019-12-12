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
#include <cstdlib>
#include <cctype>
#include <cstring>
#include "board_common.h"
#include <xHAL/USART>
#include <xHAL/DMA>
#include <xHAL/I2C>
#include <xHAL/Shell>
#include "TaskNotificationIds.h"

void idle_thread(void *param);
void main_thread(void *param);
void servo_thread(void *param);

xHAL::DMA console_uart_tx_dma(DMA2, LL_DMA_STREAM_7, NOTIFY_USART_CONSOLE_TX_DMA_TC);
xHAL::USART console(USART1, &console_uart_tx_dma, NOTIFY_USART_CONSOLE_TXE, NOTIFY_USART_CONSOLE_RXNE);
xHAL::I2C xI2C1(I2C1, NOTIFY_I2C1);

char *next_arg(char *&arg_start) {
    while (isspace(*arg_start)) // remove extra spaces
        ++arg_start;
    char *arg_end = arg_start;
    while (*arg_end && *arg_end != ' ')
        ++arg_end;
    return arg_end;
}

int i2c1_cmd_handler(char *arg) {
    char *arg_end = arg;
    u8 addr = strtoul(arg, &arg, 0);
    bool isRead = strtoul(arg, &arg, 0);
    u16 len = strtoul(arg, &arg, 0);
    bool genStop = strtoul(arg, &arg, 0);
    
    u8 data[32];
    arg_end = arg;
    if (!isRead) for (i16 i=0; i!=len; ++i) {
        arg = arg_end; arg_end = next_arg(arg); if (arg_end == arg) { console.printf("invalid format\n"); return 1; }
        data[i] = strtoul(arg, nullptr, 0);
    }
    u16 processed = xI2C1.startTransaction(addr, isRead, data, len, genStop);
    console.printf("processed %d bytes\n", processed);
    if (isRead) {
        for (u16 i=0; i!=processed; ++i)
            console.printf("%02x ", data[i]);
        console.printf("\n");
    }
    return 0;
}

TaskHandle_t tasklist[8], *tasklistend = tasklist;
int taskControl_cmd_handler(char *arg) {
    char *arg_end = arg;
    arg = arg_end; arg_end = next_arg(arg); if (arg_end == arg) { console.printf("invalid format\n"); return 1; }
    char *argv1 = arg;
    u16 arg1len = arg_end - arg;
    arg = arg_end; arg_end = next_arg(arg); if (arg_end == arg) { console.printf("invalid format\n"); return 1; }
    char *argv2 = arg;
    u16 arg2len = arg_end - arg;

    TaskHandle_t targetTask = nullptr;
    for (TaskHandle_t *p = tasklist; p!=tasklistend; ++p){
        char *name = pcTaskGetName(*p);
        if (strlen(name) == arg2len && strncmp(name, argv2, arg2len) == 0) {
            targetTask = *p;
            break;
        }
    }
    if (!targetTask) {
        console.printf("err: task \"%.*s\" does not exist\n", arg2len, argv2);
        return 1;
    }

    if (strlen("start") == arg1len && strncmp("start", argv1, arg1len) == 0) {
        vTaskResume(targetTask);
    } else if (strlen("pause") == arg1len && strncmp("pause", argv1, arg1len) == 0) {
        vTaskSuspend(targetTask);
    } else {
        console.printf("err: invalid command \"%.*s\"\n", arg1len, argv1);
        return 1;
    }
    return 0;
}

int setpwm_cmd_handler(char *arg) {
    char *arg_end = arg;
    arg = arg_end; arg_end = next_arg(arg); if (arg_end == arg) { console.printf("invalid format\n"); return 1; }
    char *argv1 = arg;
    u16 arg1len = arg_end - arg;
    arg = arg_end; arg_end = next_arg(arg); if (arg_end == arg) { console.printf("invalid format\n"); return 1; }
    char *argv2 = arg;
    u16 arg2len = arg_end - arg;

    if (strlen("arr") == arg1len && strncmp("arr", argv1, arg1len) == 0) {
        LL_TIM_SetAutoReload(TIM14, atoi(argv2));
    } else if (strlen("val") == arg1len && strncmp("val", argv1, arg1len) == 0) {
        LL_TIM_OC_SetCompareCH1(TIM14, atoi(argv2));
    } else {
        console.printf("err: invalid command \"%.*s\"\n", arg1len, argv1);
        return 1;
    }
    return 0;
}

xHAL::ShellCommand cmds[] = {
    {"hello", 0, [](char *)->int { console.printf("hello this is shell\n"); return 0; } },
    {"rx_overflow", 0, [](char *)->int { console.printf("error: rx_overflow\n"); return 1; } },
    {"i2c1", 0, &i2c1_cmd_handler},
    {"task", 0, &taskControl_cmd_handler},
    {"setpwm", 0, &setpwm_cmd_handler},
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


    LL_EXTI_DisableIT_0_31(LL_EXTI_LINE_0);
    MX_DMA_Init();
    MX_GPIO_Init();
    MX_I2C1_Init();
    MX_TIM14_Init();
    MX_USART1_UART_Init();
    LL_DMA_SetPeriphAddress(DMA2, LL_DMA_STREAM_7, LL_USART_DMA_GetRegAddr(USART1));
    LL_GPIO_ResetOutputPin(GPIOE, LL_GPIO_PIN_1);
    delay_micros(100000);

    enable_cycle_counter();

    LL_USART_EnableIT_RXNE(USART1);

    //xTaskCreate(idle_thread, "idle", 256, nullptr, 0, nullptr);
    xTaskCreate(main_thread, "main", 256, nullptr, 10, tasklistend++);
    xTaskCreate(servo_thread, "servo", 256, nullptr, 30, tasklistend++);
    xTaskCreate([](void*) { shell.run(); }, "shell", 256, nullptr, 50, tasklistend++);

    LL_GPIO_SetOutputPin(GPIOE, LL_GPIO_PIN_1);

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
    
    u16 val = 0;
    auto dev = TIM14;
    LL_TIM_SetAutoReload(dev, 2600);
    LL_TIM_EnableARRPreload(dev);
    LL_TIM_SetPrescaler(dev, get_cpu_frequency() / 2 / 1000000 - 1);
    LL_TIM_OC_SetCompareCH1(dev, val);
    LL_TIM_CC_EnableChannel(dev, LL_TIM_CHANNEL_CH1);
    LL_TIM_EnableCounter(dev);
    while (1) {
        val += 5;
        if (val > LL_TIM_GetAutoReload(dev)) val = 0;
        console.printf("[%s] STACK_UNUSED = %3" PRIu32 " val = %" PRIu16 "\n", pcTaskGetName(thisTask), uxTaskGetStackHighWaterMark(thisTask), val);
        LL_TIM_OC_SetCompareCH1(dev, val);
        vTaskDelay(10);
    }
}

void idle_thread(void *param) {
    vTaskDelete(nullptr);
}

void Error_Handler(void) {
    console.printf("ST HAL Error_handler\n");
    FailAndInfiniteLoop();
}