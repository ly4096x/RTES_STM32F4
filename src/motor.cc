#include <xHAL/USART>
#include <stm32f4xx_ll_tim.h>
#include <stm32f4xx_ll_gpio.h>
#include <FreeRTOS.h>
#include <cinttypes>
#include <main.h>

extern xHAL::USART console;
f32 pid_param[] = {80, 600, 0};
f32 target_speed_rpm = 0;
TIM_TypeDef *motor_timer = TIM12, *encoders[2] = {TIM3, TIM4}, *pid_timer = TIM7;
i32 motor_dir[2] = {-1, -1}, encoder_dir[2] = {-1, 1};

void initencoder(TIM_TypeDef *dev) {
    LL_TIM_SetAutoReload(dev, 0xffff);
    LL_TIM_EnableCounter(dev);
}

void init_motor_timer() {
    LL_TIM_SetPrescaler(motor_timer, get_cpu_frequency() / 2 / 1000000 - 1);
    LL_TIM_SetAutoReload(motor_timer, 2500);
    LL_TIM_EnableARRPreload(motor_timer);

    LL_TIM_OC_SetCompareCH1(motor_timer, 1500);
    LL_TIM_OC_SetCompareCH2(motor_timer, 1500);
    LL_TIM_CC_EnableChannel(motor_timer, LL_TIM_CHANNEL_CH1);
    LL_TIM_CC_EnableChannel(motor_timer, LL_TIM_CHANNEL_CH2);
    LL_TIM_EnableCounter(motor_timer);
}

void motor_thread(void *param) {
    initencoder(encoders[0]);
    initencoder(encoders[1]);
    init_motor_timer();

    LL_TIM_SetPrescaler(pid_timer, get_cpu_frequency() / 1000000 / 2 - 1);
    LL_TIM_SetAutoReload(pid_timer, 1000000 / 200 - 1);
    LL_TIM_EnableARRPreload(pid_timer);
    LL_TIM_EnableCounter(pid_timer);
    LL_TIM_EnableIT_UPDATE(pid_timer);

    vTaskSuspend(nullptr);

    u16 encoderVals[2][2] = {0};
    f32 speeds[2][2];
    while (1) {
        for (i32 i=0; i!=2; ++i) {
            auto &encoderVal = encoderVals[i];
            auto &speed = speeds[i];
            
        encoderVal[0] = encoderVal[1];
        encoderVal[1] = LL_TIM_GetCounter(encoders[i]);
        speed[0] = speed[1];
        speed[1] = (i16)(encoderVal[1] - encoderVal[0]) / 1560.f * 60 * 100;
        f32 accel = (speed[1] - speed[0]) / 60 / 10e-3f;
        console.printf("[mot%d] pos = %5" PRIu16 " speed = %6.2frpm accel = %6.2fr/s^2 time = %9" PRIu32 "ms\n", i, encoderVal[0], speed[1], accel, getSysTickCount() * 10);
        }
        
        vTaskDelay(1);
    }
}

extern "C"
void TIM7_IRQHandler() {
    LL_TIM_ClearFlag_UPDATE(TIM7);

    f32 dt = 1.f / 200.f;
    f32 &kp = pid_param[0], &ki = pid_param[1], &kd = pid_param[2];

    static u16 encoderVals[2][2] = {0};
    static f32 speeds[2], integrals[2] = {0};
    for (u32 i = 0; i != 2; ++i) {
        auto &encoderVal = encoderVals[i];
        encoderVal[0] = encoderVal[1];
        encoderVal[1] = LL_TIM_GetCounter(encoders[i]);
    }

    i16 pwm[2];
    if (-0.0001f < target_speed_rpm && target_speed_rpm < 0.0001f){
        pwm[0] = 0;
        pwm[1] = 0;
    } else for (u32 i = 0; i != 2; ++i) {
        auto &speed = speeds[i];
        auto &encoderVal = encoderVals[i];
        auto &integral = integrals[i];

        speed = pid_param[3] * (i16)(encoderVal[1] - encoderVal[0]) / 1560.f / dt * 60 * encoder_dir[i];
        
        f32 err_rps = (target_speed_rpm - speed) / 60;
        
        integral += err_rps * dt;
        f32 P = kp * err_rps;
        f32 I = ki * integral;
        f32 D = kd * err_rps / dt;
        pwm[i] = P + I + D;
        if (I > 500) integral = 500 / ki;
        else if (I < -500) integral = -500 / ki;
        if (pwm[i] > 500) pwm[i] = 500;
        else if (pwm[i] < -500) pwm[i] = -500;
    }

    LL_TIM_OC_SetCompareCH1(motor_timer, 1500 + pwm[0] * motor_dir[0]);
    LL_TIM_OC_SetCompareCH2(motor_timer, 1500 + pwm[1] * motor_dir[1]);

}