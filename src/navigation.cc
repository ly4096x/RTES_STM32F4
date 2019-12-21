#include <xHAL/USART>
#include <FreeRTOS.h>
#include <cinttypes>
#include <cmath>
#include <stm32f4xx_ll_tim.h>
#include <vl53l1x-arduino/VL53L1X.h>

extern xHAL::USART console;

/*
durationFor90 : the time needed to make a 90 deg turn
durationFor1m : the time needed to go 1 meter in a straight line
demo_rpm_turn : motor RPM for turning
demo_rpm_straight : motor RPM for going in straight line
demo_servo_delay : delay factor for servo motion
demo_rssi_threshould : RSSI threshould for making navigation decision
*/
f32 durationFor90 = 1.95f, durationFor1m = 5.f, demo_rpm_turn = 40.f, demo_rpm_straight = 120;
u32 demo_servo_delay = 25;
i32 demo_rssi_threshould = 2;


u32 move_until = 0;
u8 move_mode = 0;

enum : u8 {
    STOP,
    LEFT,
    RIGHT,
    PRE_FORWARD,
    FORWARD,
    BACK,
    AVOID
};

void move_car(const f32 rpm_l, const f32 rpm_r, const u32 duration) {
    extern volatile f32 target_speed_rpm[2];
    target_speed_rpm[0] = rpm_l;
    target_speed_rpm[1] = rpm_r;
    if (duration == 0) move_until = 0;
    else move_until = getSysTickCount() + duration;
}

void stop_car() {
    extern volatile f32 target_speed_rpm[2];
    target_speed_rpm[0] = 0;
    target_speed_rpm[1] = 0;
    move_until = 0;
    move_mode = STOP;
}

void move_left(const u16 degree, const bool noStop) {
    u32 duration = degree / 90.f * durationFor90 * configTICK_RATE_HZ;
    move_car(-demo_rpm_turn, demo_rpm_turn, noStop ? 0 : duration);
    move_mode = LEFT;
}
void move_right(const u16 degree, const bool noStop) {
    u32 duration = degree / 90.f * durationFor90 * configTICK_RATE_HZ;
    move_car(demo_rpm_turn, -demo_rpm_turn, noStop ? 0 : duration);
    move_mode = RIGHT;
}
void move_forward(const f32 length_meter, const bool noStop) {
    u32 duration = length_meter * durationFor1m * configTICK_RATE_HZ;
    move_car(demo_rpm_straight, demo_rpm_straight, noStop ? 0 : duration);
    move_mode = FORWARD;
}
void move_back(const f32 length_meter, const bool noStop) {
    u32 duration = length_meter * durationFor1m * configTICK_RATE_HZ;
    move_car(-demo_rpm_straight, -demo_rpm_straight, noStop ? 0 : duration);
    move_mode = BACK;
}

template<typename ty>
inline ty abs(const ty &v) { return v >= 0 ? v : -v; }
template<typename ty>
inline ty max(const ty &x, const ty &y) { return x > y ? x : y; }
template<typename ty>
inline ty min(const ty &x, const ty &y) { return x < y ? x : y; }

i16 servo_current_degree = 0;

void set_servo(const i16 degree) {
    TIM_TypeDef *dev = TIM14;
    u16 pwm = 1400 + (2500 - 300) / 2.f * -degree / 90.f;
    LL_TIM_OC_SetCompareCH1(dev, pwm);
    vTaskDelay(abs(servo_current_degree - degree) / 180.f * demo_servo_delay);
    servo_current_degree = degree;
}

u16 get_range() {
    extern VL53L1X *sensor_ptr;
    u16 distance;
    do {
        distance = sensor_ptr->read();
        if (sensor_ptr->timeoutOccurred())
            continue;
    } while (false);
    //console.printf("[nav] range=%4d mode=%2d\n", distance, move_mode);
    return distance;
}

// keep doing the current motion and stop until the measured range is larger than limit
inline void stop_when_range_larger(const u16 range) {
    while (get_range() < range);
    stop_car();
}

// get the least range limit
const f32 get_least_range() {
    static constexpr f32 PI = 3.14159265359f;
    const f32 xlim = servo_current_degree == 0 ? 1e6 : (200 / sinf(abs(servo_current_degree) * PI / 180));
    const f32 ylim = abs(servo_current_degree) == 90 ? 1e6 : (400 / cosf(abs(servo_current_degree) * PI / 180));
    const f32 least_range = min(xlim, ylim);
    return least_range;
}

// avoid obstacles by rotating
void avoid_obstacle(i8 &dir) {
    const f32 least_range = get_least_range();
    if (get_range() < least_range) {
        if (!dir) dir = servo_current_degree < 0 ? 1 : -1;
        
        if (dir == 1) move_right(0, true);
        else if (dir == -1) move_left(0, true);

        stop_when_range_larger(least_range);
    }
}

// see if able to find a 40x40cm movable field. if not, do obstacle avoidance
void clear_path() {
    for (i16 deg = -90; deg <= 90; deg += 10) {
        set_servo(deg);
        i8 dir = 0;
        avoid_obstacle(dir);
    }
}

extern xHAL::USART console;
void navigation_thread(void *param) {
    //vTaskSuspend(nullptr);
    vTaskDelay(5 * configTICK_RATE_HZ);

    i32 servo_dir = 10;
    move_mode = PRE_FORWARD;
    i16 rssiq[3], rssi_count = 0;
    u32 rssiTimestamp = 0;

    while (true) {
        if (move_until != 0 && move_until - getSysTickCount() <= 0)
            stop_car();
        
        switch (move_mode) {
        case STOP:
            vTaskSuspend(nullptr);
            break;
        case PRE_FORWARD:
            clear_path();
            move_forward(0, 1);
            rssi_count = 0;
            break;
        case FORWARD:
            if (servo_current_degree >= 90) servo_dir = -10;
            else if (servo_current_degree <= -90) servo_dir = 10;

            extern i16 rssi[2];
            if (rssi[0] > -30) {
                move_mode = STOP;
                stop_car();
                break;
            } else if (rssi_count == 0) {
                rssiq[rssi_count++] = rssi[0];
                rssiTimestamp = getSysTickCount();
            } else if (rssi_count == 1 && getSysTickCount() - rssiTimestamp >= 2 * configTICK_RATE_HZ) {
                rssiq[rssi_count++] = rssi[0];
            } else if (rssi_count == 2 && getSysTickCount() - rssiTimestamp >= 4 * configTICK_RATE_HZ) {
                rssiq[rssi_count] = rssi[0];
                rssi_count = 0;
                console.printf("[nav] %3d %3d %3d\n", rssiq[0], rssiq[1], rssiq[2]);
                if (rssiq[0] - rssiq[1] > demo_rssi_threshould && rssiq[1] - rssiq[2] > demo_rssi_threshould) {
                    move_right(180, false);
                    vTaskDelay(move_until - getSysTickCount());
                    stop_car();
                    move_mode = PRE_FORWARD;
                    break;
                } else if (rssiq[0] - rssiq[1] < demo_rssi_threshould && rssiq[1] - rssiq[2] < demo_rssi_threshould) {

                } else {
                    move_right(90, false);
                    vTaskDelay(move_until - getSysTickCount());
                    stop_car();
                    move_mode = PRE_FORWARD;
                    break;
                }
            }

            set_servo(servo_current_degree + servo_dir);
            if (get_range() < get_least_range()) {
                stop_car();
                move_mode = PRE_FORWARD;
            }
            break;
        default:
            break;
        }
    }
}