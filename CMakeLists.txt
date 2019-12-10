PROJECT(RTES_STM32F4)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
ENABLE_LANGUAGE(C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-comment -Wno-register -Wno-write-strings")

#FIND_PACKAGE(CMSIS REQUIRED)
#FIND_PACKAGE(STM32HAL COMPONENTS gpio REQUIRED)

include_directories(
    CubeMX/Core/Inc
    CubeMX/Middlewares/Third_Party/FreeRTOS/Source/include
    CubeMX/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2
    CubeMX/Drivers/CMSIS/Include
    CubeMX/Drivers/CMSIS/Include
    CubeMX/Drivers/STM32F4xx_HAL_Driver/Inc
    CubeMX/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F
    CubeMX/Drivers/CMSIS/Device/ST/STM32F4xx/Include
)
file(GLOB CubeMX_SRCS
    CubeMX/Core/Src/*.c
    CubeMX/Middlewares/Third_Party/FreeRTOS/Source/*.c
    CubeMX/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/cmsis_os2.c
    CubeMX/Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/heap_4.c
    CubeMX/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F/port.c
    CubeMX/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_gpio.c
    CubeMX/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_adc.c
    CubeMX/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_dma.c
    CubeMX/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_rcc.c
    CubeMX/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_utils.c
    CubeMX/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_exti.c
    CubeMX/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_spi.c
    CubeMX/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_usart.c
    CubeMX/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_i2c.c
    CubeMX/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_tim.c
    CubeMX/startup_stm32f407xx.s
)
file(GLOB_RECURSE ArduinoAPI_SRCS
    lib/ArduinoAPI/*.c
    lib/ArduinoAPI/*.cc
    lib/ArduinoAPI/*.cxx
    lib/ArduinoAPI/*.cpp
)
file(GLOB_RECURSE xHAL_SRCS
    lib/xHAL/*.c
    lib/xHAL/*.cc
    lib/xHAL/*.cxx
    lib/xHAL/*.cpp
)

include_directories(
    include
    lib
    lib/ArduinoAPI
    ${CubeMX_INCLUDES}
)

file(GLOB SRCS
    src/*.c
    src/*.cc
    src/*.cpp
    src/*.cxx
    #lib/Adafruit-PWM-Servo-Driver-Library/Adafruit_PWMServoDriver.cpp
)

ADD_EXECUTABLE(${CMAKE_PROJECT_NAME} ${SRCS} ${CubeMX_SRCS} ${xHAL_SRCS} ${ArduinoAPI_SRCS})

SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -specs=nosys.specs")

STM32_SET_TARGET_PROPERTIES(${CMAKE_PROJECT_NAME})
get_target_property(tmpvar ${CMAKE_PROJECT_NAME} COMPILE_DEFINITIONS)
SET_TARGET_PROPERTIES(${CMAKE_PROJECT_NAME} PROPERTIES COMPILE_DEFINITIONS "${tmpvar};USE_FULL_LL_DRIVER;HSE_VALUE=8000000;HSE_STARTUP_TIMEOUT=100;LSE_STARTUP_TIMEOUT=5000;LSE_VALUE=32768;VDD_VALUE=3300;PREFETCH_ENABLE=1;EXTERNAL_CLOCK_VALUE=12288000;HSI_VALUE=16000000;LSI_VALUE=32000;INSTRUCTION_CACHE_ENABLE=1;DATA_CACHE_ENABLE=1;STM32F407;__weak=__attribute__((weak))")
STM32_ADD_HEX_BIN_TARGETS(${CMAKE_PROJECT_NAME})
add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_SIZE} --format=berkeley "${CMAKE_PROJECT_NAME}")
