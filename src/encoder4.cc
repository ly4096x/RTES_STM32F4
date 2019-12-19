#include <xHAL/USART>
#include <stm32f4xx_ll_tim.h>
#include <FreeRTOS.h>
#include <cinttypes>

extern xHAL::USART console;
f32 pid_param[4] = {30, 100, 0, 1};
f32 target_speed_rpm = 0;

void encoder_tim4_thread(void *param) {
    LL_TIM_SetAutoReload(TIM4, 0xffff);
    LL_TIM_EnableCounter(TIM4);

    LL_TIM_OC_SetCompareCH1(TIM14, 1500);
    LL_TIM_SetAutoReload(TIM7, 84000000 / 200 - 1);
    LL_TIM_EnableARRPreload(TIM7);
    LL_TIM_EnableCounter(TIM7);
    LL_TIM_EnableIT_UPDATE(TIM7);

    vTaskSuspend(nullptr);

    u16 encoderVal[2] = {0};
    f32 speed[2];
    while (1) {
        encoderVal[0] = encoderVal[1];
        encoderVal[1] = LL_TIM_GetCounter(TIM4);
        speed[0] = speed[1];
        speed[1] = (i16)(encoderVal[1] - encoderVal[0]) / 1560.f * 60 * 100;
        f32 accel = (speed[1] - speed[0]) / 60 / 10e-3f;

        console.printf("[encoder4] pos = %5" PRIu16 " speed = %6.2frpm accel = %6.2fr/s^2 time = %9" PRIu32 "ms\n", encoderVal[0], speed[1], accel, getSysTickCount() * 10);
        vTaskDelay(1);
    }
}

extern "C"
void TIM7_IRQHandler() {
    LL_TIM_ClearFlag_UPDATE(TIM7);
    static u32 cyccnt = get_cycle_counter_value();

    //f32 dt = 1.f / 200.f;
    f32 dt = (get_cycle_counter_value() - cyccnt) * 1.f / get_cpu_frequency();
    cyccnt = get_cycle_counter_value();
    static u16 encoderVal[2] = {0};
    static f32 speed, integral = 0;
    encoderVal[0] = encoderVal[1];
    encoderVal[1] = LL_TIM_GetCounter(TIM4);
    speed = pid_param[3] * (i16)(encoderVal[1] - encoderVal[0]) / 1560.f / dt * 60;

    f32 &kp = pid_param[0], &ki = pid_param[1], &kd = pid_param[2];
    f32 err_rps = (target_speed_rpm - speed) / 60;

    integral += err_rps * dt;
    f32 P = kp * err_rps;
    f32 I = ki * integral;
    f32 D = kd * err_rps / dt;
    f32 sum = P + I + D;
    if (I > 500) integral = 500 / ki;
    else if (I < -500) integral = -500 / ki;
    if (sum > 500) sum = 500;
    else if (sum < -500) sum = -500;
    LL_TIM_OC_SetCompareCH1(TIM14, 1500 + sum);
}