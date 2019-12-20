extern "C" {
#include <FreeRTOS.h>
#include <task.h>
#include <stm32f4xx_ll_dma.h>
#include <stm32f4xx_ll_usart.h>
#include <stm32f4xx_ll_i2c.h>
#include "TaskNotificationIds.h"
}
#include "board_common.h"
#include <xHAL/USART>
#include <xHAL/I2C>
#include <cstring>

extern "C" {
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName);
void vApplicationMallocFailedHook(void);

void NMI_Handler                   (void);
void HardFault_Handler             (void);
void MemManage_Handler             (void);
void BusFault_Handler              (void);
void UsageFault_Handler            (void);
void SVC_Handler                   (void);
void DebugMon_Handler              (void);
void PendSV_Handler                (void);
void SysTick_Handler               (void);
void WWDG_IRQHandler               (void);
void PVD_IRQHandler                (void);
void TAMP_STAMP_IRQHandler         (void);
void RTC_WKUP_IRQHandler           (void);
void FLASH_IRQHandler              (void);
void RCC_IRQHandler                (void);
void EXTI0_IRQHandler              (void);
void EXTI1_IRQHandler              (void);
void EXTI2_IRQHandler              (void);
void EXTI3_IRQHandler              (void);
void EXTI4_IRQHandler              (void);
void DMA1_Stream0_IRQHandler       (void);
void DMA1_Stream1_IRQHandler       (void);
void DMA1_Stream2_IRQHandler       (void);
void DMA1_Stream3_IRQHandler       (void);
void DMA1_Stream4_IRQHandler       (void);
void DMA1_Stream5_IRQHandler       (void);
void DMA1_Stream6_IRQHandler       (void);
void ADC_IRQHandler                (void);
void CAN1_TX_IRQHandler            (void);
void CAN1_RX0_IRQHandler           (void);
void CAN1_RX1_IRQHandler           (void);
void CAN1_SCE_IRQHandler           (void);
void EXTI9_5_IRQHandler            (void);
void TIM1_BRK_TIM9_IRQHandler      (void);
void TIM1_UP_TIM10_IRQHandler      (void);
void TIM1_TRG_COM_TIM11_IRQHandler (void);
void TIM1_CC_IRQHandler            (void);
void TIM2_IRQHandler               (void);
void TIM3_IRQHandler               (void);
void TIM4_IRQHandler               (void);
void I2C1_EV_IRQHandler            (void);
void I2C1_ER_IRQHandler            (void);
void I2C2_EV_IRQHandler            (void);
void I2C2_ER_IRQHandler            (void);
void SPI1_IRQHandler               (void);
void SPI2_IRQHandler               (void);
void USART1_IRQHandler             (void);
void USART2_IRQHandler             (void);
void USART3_IRQHandler             (void);
void EXTI15_10_IRQHandler          (void);
void RTC_Alarm_IRQHandler          (void);
void OTG_FS_WKUP_IRQHandler        (void);
void TIM8_BRK_TIM12_IRQHandler     (void);
void TIM8_UP_TIM13_IRQHandler      (void);
void TIM8_TRG_COM_TIM14_IRQHandler (void);
void TIM8_CC_IRQHandler            (void);
void DMA1_Stream7_IRQHandler       (void);
void FSMC_IRQHandler               (void);
void SDIO_IRQHandler               (void);
void TIM5_IRQHandler               (void);
void SPI3_IRQHandler               (void);
void UART4_IRQHandler              (void);
void UART5_IRQHandler              (void);
void TIM6_DAC_IRQHandler           (void);
void TIM7_IRQHandler               (void);
void DMA2_Stream0_IRQHandler       (void);
void DMA2_Stream1_IRQHandler       (void);
void DMA2_Stream2_IRQHandler       (void);
void DMA2_Stream3_IRQHandler       (void);
void DMA2_Stream4_IRQHandler       (void);
void ETH_IRQHandler                (void);
void ETH_WKUP_IRQHandler           (void);
void CAN2_TX_IRQHandler            (void);
void CAN2_RX0_IRQHandler           (void);
void CAN2_RX1_IRQHandler           (void);
void CAN2_SCE_IRQHandler           (void);
void OTG_FS_IRQHandler             (void);
void DMA2_Stream5_IRQHandler       (void);
void DMA2_Stream6_IRQHandler       (void);
void DMA2_Stream7_IRQHandler       (void);
void USART6_IRQHandler             (void);
void I2C3_EV_IRQHandler            (void);
void I2C3_ER_IRQHandler            (void);
void OTG_HS_EP1_OUT_IRQHandler     (void);
void OTG_HS_EP1_IN_IRQHandler      (void);
void OTG_HS_WKUP_IRQHandler        (void);
void OTG_HS_IRQHandler             (void);
void DCMI_IRQHandler               (void);
void HASH_RNG_IRQHandler           (void);
void FPU_IRQHandler                (void);
}

void DefaultHardwareExceptionHandler(const char *funcName);

void NMI_Handler                       () { DefaultHardwareExceptionHandler(__func__); }
void HardFault_Handler                 () { DefaultHardwareExceptionHandler(__func__); }
void MemManage_Handler                 () { DefaultHardwareExceptionHandler(__func__); }
void BusFault_Handler                  () { DefaultHardwareExceptionHandler(__func__); }
void UsageFault_Handler                () { DefaultHardwareExceptionHandler(__func__); } //\
void SVC_Handler                       () { DefaultHardwareExceptionHandler(__func__); } // RTOS
void DebugMon_Handler                  () { DefaultHardwareExceptionHandler(__func__); } //\
void PendSV_Handler                    () { DefaultHardwareExceptionHandler(__func__); } // RTOS \
void SysTick_Handler                   () { DefaultHardwareExceptionHandler(__func__); } // RTOS
void WWDG_IRQHandler                   () { DefaultHardwareExceptionHandler(__func__); }
void PVD_IRQHandler                    () { DefaultHardwareExceptionHandler(__func__); }
void TAMP_STAMP_IRQHandler             () { DefaultHardwareExceptionHandler(__func__); }
void RTC_WKUP_IRQHandler               () { DefaultHardwareExceptionHandler(__func__); }
void FLASH_IRQHandler                  () { DefaultHardwareExceptionHandler(__func__); }
void RCC_IRQHandler                    () { DefaultHardwareExceptionHandler(__func__); }
void EXTI0_IRQHandler                  () { DefaultHardwareExceptionHandler(__func__); }
void EXTI1_IRQHandler                  () { DefaultHardwareExceptionHandler(__func__); }
void EXTI2_IRQHandler                  () { DefaultHardwareExceptionHandler(__func__); }
void EXTI3_IRQHandler                  () { DefaultHardwareExceptionHandler(__func__); }
void EXTI4_IRQHandler                  () { DefaultHardwareExceptionHandler(__func__); }
void DMA1_Stream0_IRQHandler           () { DefaultHardwareExceptionHandler(__func__); }
void DMA1_Stream1_IRQHandler           () { DefaultHardwareExceptionHandler(__func__); }
void DMA1_Stream2_IRQHandler           () { DefaultHardwareExceptionHandler(__func__); }
void DMA1_Stream3_IRQHandler           () { DefaultHardwareExceptionHandler(__func__); }
void DMA1_Stream4_IRQHandler           () { DefaultHardwareExceptionHandler(__func__); }
void DMA1_Stream5_IRQHandler           () { DefaultHardwareExceptionHandler(__func__); }
void DMA1_Stream6_IRQHandler           () { DefaultHardwareExceptionHandler(__func__); }
void ADC_IRQHandler                    () { DefaultHardwareExceptionHandler(__func__); }
void CAN1_TX_IRQHandler                () { DefaultHardwareExceptionHandler(__func__); }
void CAN1_RX0_IRQHandler               () { DefaultHardwareExceptionHandler(__func__); }
void CAN1_RX1_IRQHandler               () { DefaultHardwareExceptionHandler(__func__); }
void CAN1_SCE_IRQHandler               () { DefaultHardwareExceptionHandler(__func__); }
void EXTI9_5_IRQHandler                () { DefaultHardwareExceptionHandler(__func__); }
void TIM1_BRK_TIM9_IRQHandler          () { DefaultHardwareExceptionHandler(__func__); }
void TIM1_UP_TIM10_IRQHandler          () { DefaultHardwareExceptionHandler(__func__); }
void TIM1_TRG_COM_TIM11_IRQHandler     () { DefaultHardwareExceptionHandler(__func__); }
void TIM1_CC_IRQHandler                () { DefaultHardwareExceptionHandler(__func__); }
void TIM2_IRQHandler                   () { DefaultHardwareExceptionHandler(__func__); }
void TIM3_IRQHandler                   () { DefaultHardwareExceptionHandler(__func__); }
void TIM4_IRQHandler                   () { DefaultHardwareExceptionHandler(__func__); } //\
void I2C1_EV_IRQHandler                () { DefaultHardwareExceptionHandler(__func__); } // I2C1 \
void I2C1_ER_IRQHandler                () { DefaultHardwareExceptionHandler(__func__); } // I2C1
void I2C2_EV_IRQHandler                () { DefaultHardwareExceptionHandler(__func__); }
void I2C2_ER_IRQHandler                () { DefaultHardwareExceptionHandler(__func__); }
void SPI1_IRQHandler                   () { DefaultHardwareExceptionHandler(__func__); }
void SPI2_IRQHandler                   () { DefaultHardwareExceptionHandler(__func__); } //\
void USART1_IRQHandler                 () { DefaultHardwareExceptionHandler(__func__); } // USART1
void USART2_IRQHandler                 () { DefaultHardwareExceptionHandler(__func__); }
void USART3_IRQHandler                 () { DefaultHardwareExceptionHandler(__func__); }
void EXTI15_10_IRQHandler              () { DefaultHardwareExceptionHandler(__func__); }
void RTC_Alarm_IRQHandler              () { DefaultHardwareExceptionHandler(__func__); }
void OTG_FS_WKUP_IRQHandler            () { DefaultHardwareExceptionHandler(__func__); }
void TIM8_BRK_TIM12_IRQHandler         () { DefaultHardwareExceptionHandler(__func__); }
void TIM8_UP_TIM13_IRQHandler          () { DefaultHardwareExceptionHandler(__func__); }
void TIM8_TRG_COM_TIM14_IRQHandler     () { DefaultHardwareExceptionHandler(__func__); }
void TIM8_CC_IRQHandler                () { DefaultHardwareExceptionHandler(__func__); }
void DMA1_Stream7_IRQHandler           () { DefaultHardwareExceptionHandler(__func__); }
void FSMC_IRQHandler                   () { DefaultHardwareExceptionHandler(__func__); }
void SDIO_IRQHandler                   () { DefaultHardwareExceptionHandler(__func__); }
void TIM5_IRQHandler                   () { DefaultHardwareExceptionHandler(__func__); }
void SPI3_IRQHandler                   () { DefaultHardwareExceptionHandler(__func__); } // \
void UART4_IRQHandler                  () { DefaultHardwareExceptionHandler(__func__); } // UART4 \
void UART5_IRQHandler                  () { DefaultHardwareExceptionHandler(__func__); } // UART5
void TIM6_DAC_IRQHandler               () { DefaultHardwareExceptionHandler(__func__); } // \
void TIM7_IRQHandler                   () { DefaultHardwareExceptionHandler(__func__); } // TIM7
void DMA2_Stream0_IRQHandler           () { DefaultHardwareExceptionHandler(__func__); }
void DMA2_Stream1_IRQHandler           () { DefaultHardwareExceptionHandler(__func__); }
void DMA2_Stream2_IRQHandler           () { DefaultHardwareExceptionHandler(__func__); }
void DMA2_Stream3_IRQHandler           () { DefaultHardwareExceptionHandler(__func__); }
void DMA2_Stream4_IRQHandler           () { DefaultHardwareExceptionHandler(__func__); }
void ETH_IRQHandler                    () { DefaultHardwareExceptionHandler(__func__); }
void ETH_WKUP_IRQHandler               () { DefaultHardwareExceptionHandler(__func__); }
void CAN2_TX_IRQHandler                () { DefaultHardwareExceptionHandler(__func__); }
void CAN2_RX0_IRQHandler               () { DefaultHardwareExceptionHandler(__func__); }
void CAN2_RX1_IRQHandler               () { DefaultHardwareExceptionHandler(__func__); }
void CAN2_SCE_IRQHandler               () { DefaultHardwareExceptionHandler(__func__); }
void OTG_FS_IRQHandler                 () { DefaultHardwareExceptionHandler(__func__); }
void DMA2_Stream5_IRQHandler           () { DefaultHardwareExceptionHandler(__func__); }
void DMA2_Stream6_IRQHandler           () { DefaultHardwareExceptionHandler(__func__); } //\
void DMA2_Stream7_IRQHandler           () { DefaultHardwareExceptionHandler(__func__); } // USART1_TX
void USART6_IRQHandler                 () { DefaultHardwareExceptionHandler(__func__); }
void I2C3_EV_IRQHandler                () { DefaultHardwareExceptionHandler(__func__); }
void I2C3_ER_IRQHandler                () { DefaultHardwareExceptionHandler(__func__); }
void OTG_HS_EP1_OUT_IRQHandler         () { DefaultHardwareExceptionHandler(__func__); }
void OTG_HS_EP1_IN_IRQHandler          () { DefaultHardwareExceptionHandler(__func__); }
void OTG_HS_WKUP_IRQHandler            () { DefaultHardwareExceptionHandler(__func__); }
void OTG_HS_IRQHandler                 () { DefaultHardwareExceptionHandler(__func__); }
void DCMI_IRQHandler                   () { DefaultHardwareExceptionHandler(__func__); }
void HASH_RNG_IRQHandler               () { DefaultHardwareExceptionHandler(__func__); }
void FPU_IRQHandler                    () { DefaultHardwareExceptionHandler(__func__); }


extern xHAL::USART console;

void DefaultHardwareExceptionHandler(const char *funcName) {
    static constexpr char HARDWARE_EXCEPTION_HEADER[] = "\nHardException: ";
    console.writeWithPolling((u8*)HARDWARE_EXCEPTION_HEADER, strlen(HARDWARE_EXCEPTION_HEADER));
    console.writeWithPolling((u8*)funcName, strlen(funcName));
    console.writeWithPolling((u8*)"\n", 1);
    FailAndInfiniteLoop();
}

void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName) {
    char *msg1 = "RTOS error: stack overflow \"", *msg2 = "\"\n";
    console.writeWithPolling((u8*)msg1, strlen(msg1));
    console.writeWithPolling((u8*)pcTaskName, strlen((const char *)pcTaskName));
    console.writeWithPolling((u8*)msg2, strlen(msg2));
    FailAndInfiniteLoop();
}
void vApplicationMallocFailedHook(void) {
    char *msg1 = "RTOS error: malloc failed\n";
    console.writeWithPolling((u8*)msg1, strlen(msg1));
    FailAndInfiniteLoop();
}



// USART3_TX
void DMA2_Stream7_IRQHandler(void){
    if (LL_DMA_IsActiveFlag_TC7(DMA2) || LL_DMA_IsActiveFlag_TE7(DMA2)) {
        extern xHAL::DMA console_uart_tx_dma;
        console_uart_tx_dma.interruptHandler(LL_DMA_IsActiveFlag_TC7(DMA2), LL_DMA_IsActiveFlag_TE7(DMA2));
        LL_DMA_ClearFlag_TC7(DMA2);
        LL_DMA_ClearFlag_TE7(DMA2);
        return;
    }
    FailAndInfiniteLoop();
}

void USART1_IRQHandler(void) {
    extern xHAL::USART console;
    if (LL_USART_IsEnabledIT_TXE(USART1) && LL_USART_IsActiveFlag_TXE(USART1)) {
        console.txInterruptHandler();
        return;
    }
    if (LL_USART_IsEnabledIT_RXNE(USART1) && LL_USART_IsActiveFlag_RXNE(USART1)) {
        console.rxInterruptHandler();
        return;
    }
    FailAndInfiniteLoop();
}

void UART4_IRQHandler(void) {
    extern xHAL::USART xUART4;
    if (LL_USART_IsEnabledIT_RXNE(UART4) && LL_USART_IsActiveFlag_RXNE(UART4)) {
        xUART4.rxInterruptHandler();
        return;
    }
}

void UART5_IRQHandler(void) {
    extern xHAL::USART xUART5;
    if (LL_USART_IsEnabledIT_RXNE(UART5) && LL_USART_IsActiveFlag_RXNE(UART5)) {
        xUART5.rxInterruptHandler();
        return;
    }
}

void I2C1_EV_IRQHandler() {
    extern xHAL::I2C xI2C1;
    xI2C1.interruptHandler(0);
}
void I2C1_ER_IRQHandler() {
    extern xHAL::I2C xI2C1;
    xI2C1.interruptHandler(1);
}

extern void TIM7_IRQHandler();