#include "board_common.h"
#include <xHAL/I2C>
#include <xHAL/USART>
#include <xHAL/Shell>
#include <cctype>
#include <cstring>
#include <cinttypes>
#include <cstdlib>

extern "C" {
#include "tim.h"
}

extern xHAL::USART console;
extern xHAL::I2C xI2C1;
u32 blink_sequence[4] = {10, 30, 10, 2000};

int i2c1_cmd_handler(const u8 argc, char **argv) {
    if (argc < 5) { console.printf("too few args\n"); return 1; }

    u8 addr = strtoul(argv[1], nullptr, 0);
    bool isRead = strtoul(argv[2], nullptr, 0);
    u16 len = strtoul(argv[3], nullptr, 0);
    bool genStop = strtoul(argv[4], nullptr, 0);

    if (!isRead && argc < 5 + len) { console.printf("too few data\n"); return 2; }
    
    u8 data[32];
    if (!isRead) {
        for (i16 i=0; i!=len; ++i) {
            data[i] = strtoul(argv[i + 5], nullptr, 0);
        }
    }
    u16 processed = xI2C1.startTransaction(addr, isRead, data, len, genStop);
    console.printf("processed %d bytes\n", processed);
    if (isRead) {
        for (u16 i=0; i!=processed; ++i)
            console.printf("%02x ", data[i]);
        console.printf("\n");
    }
#if LOG_I2C_EMPTY_INT
    extern int i2c_did_nothing;
    extern u32 i2c_nothing_buf[];
    console.printf("did_nothing = %d\n", i2c_did_nothing);
    for (int i=0; i!=i2c_did_nothing; ++i)
        console.printf("0x%08x\n", i2c_nothing_buf[i]);
    i2c_did_nothing = 0;
#endif
    return 0;
}

TaskHandle_t tasklist[8], *tasklistend = tasklist;
int taskControl_cmd_handler(const u8 argc, char **argv) {
    if (argc < 3) { console.printf("too few args\n"); return 1; }
    const u8 argv1len = strlen(argv[1]), argv2len = strlen(argv[2]);

    TaskHandle_t targetTask = nullptr;
    for (TaskHandle_t *p = tasklist; p!=tasklistend; ++p){
        char *name = pcTaskGetName(*p);
        if (strlen(name) == argv2len && strncmp(name, argv[2], argv2len) == 0) {
            targetTask = *p;
            break;
        }
    }
    if (!targetTask) {
        console.printf("err: task \"%.*s\" does not exist\n", argv2len, argv[2]);
        return 1;
    }

    if (strlen("start") == argv1len && strncmp("start", argv[1], argv1len) == 0) {
        vTaskResume(targetTask);
    } else if (strlen("pause") == argv1len && strncmp("pause", argv[1], argv1len) == 0) {
        vTaskSuspend(targetTask);
    } else {
        console.printf("err: invalid command \"%.*s\"\n", argv1len, argv[2]);
        return 1;
    }
    return 0;
}

int setpwm_cmd_handler(const u8 argc, char **argv) {
    if (argc < 3) { console.printf("too few args\n"); return 1; }
    const u8 argv1len = strlen(argv[1]);

    if (strlen("arr") == argv1len && strncmp("arr", argv[1], argv1len) == 0) {
        LL_TIM_SetAutoReload(TIM14, atoi(argv[2]));
    } else if (strlen("val") == argv1len && strncmp("val", argv[1], argv1len) == 0) {
        LL_TIM_OC_SetCompareCH1(TIM14, atoi(argv[2]));
    } else {
        console.printf("err: invalid command \"%.*s\"\n", argv1len, argv[1]);
        return 1;
    }
    return 0;
}

int blink_cmd_handler(const u8 argc, char **argv) {
    if (argc < 2) { console.printf("too few args\n"); return 1; }
    const u8 argv1len = strlen(argv[1]);

    if (strlen("set") == argv1len && strncmp("set", argv[1], argv1len) == 0) {
        if (argc < 6) { console.printf("too few args\n"); return 1; }
        for (u32 i=0; i!=4; ++i) 
            blink_sequence[i] = strtoul(argv[3 + i], nullptr, 0);
    } else if (strlen("get") == argv1len && strncmp("get", argv[1], argv1len) == 0) {
        console.printf("%" PRIu32 " %" PRIu32 " %" PRIu32 " %" PRIu32 "\n",
            blink_sequence[0],
            blink_sequence[1],
            blink_sequence[2],
            blink_sequence[3]
        );
    } else {
        console.printf("err: invalid command \"%.*s\"\n", argv1len, argv[1]);
        return 1;
    }
    return 0;
}

int mem_cmd_handler(const u8 argc, char **argv) {
    if (argc < 3) { console.printf("too few args\n"); return 1; }
    const u8 argv1len = strlen(argv[1]);
    u32 val;

    u32 *addr = (u32*)strtoul(argv[2], nullptr, 0);
    if (!addr) { console.printf("invalid mem addr\n"); return 1; }

    if (strlen("write") == argv1len && strncmp("write", argv[1], argv1len) == 0) {
        if (argc < 4) { console.printf("too few args\n"); return 1; }
        val = strtoul(argv[3], nullptr, 0);
        *addr = val;
    } else if (strlen("read") == argv1len && strncmp("read", argv[1], argv1len) == 0) {
        val = *addr;
        console.printf("val = 0x%08x %d\n", val, val);
    }
    return 0;
}

int m4_cmd_handler(const u8 argc, char **argv) {
    if (argc < 2) { console.printf("too few args\n"); return 1; }
    const u8 argv1len = strlen(argv[1]);
    f32 val;

    extern f32 pid_param[];
    f32 &kp = pid_param[0], &ki = pid_param[1], &kd = pid_param[2];

    if (strlen("p") == argv1len && strncmp("p", argv[1], argv1len) == 0) {
        if (argc < 3) { console.printf("too few args\n"); return 1; }
        val = strtof(argv[2], nullptr);
        kp = val;
    } else if (strlen("i") == argv1len && strncmp("i", argv[1], argv1len) == 0) {
        if (argc < 3) { console.printf("too few args\n"); return 1; }
        val = strtof(argv[2], nullptr);
        ki = val;
    } else if (strlen("d") == argv1len && strncmp("d", argv[1], argv1len) == 0) {
        if (argc < 3) { console.printf("too few args\n"); return 1; }
        val = strtof(argv[2], nullptr);
        kd = val;
    } else if (strlen("s") == argv1len && strncmp("s", argv[1], argv1len) == 0) {
        if (argc < 4) { console.printf("too few args\n"); return 1; }
        extern volatile f32 target_speed_rpm[2];
        target_speed_rpm[0] = strtof(argv[2], nullptr);
        target_speed_rpm[1] = strtof(argv[3], nullptr);
    } else if (strlen("dir") == argv1len && strncmp("dir", argv[1], argv1len) == 0) {
        if (argc < 4) { console.printf("too few args\n"); return 1; }
        extern i32 encoder_dir[];
        encoder_dir[0] = strtol(argv[2], nullptr, 0);
        encoder_dir[1] = strtol(argv[3], nullptr, 0);
    } else if (strlen("diro") == argv1len && strncmp("diro", argv[1], argv1len) == 0) {
        if (argc < 4) { console.printf("too few args\n"); return 1; }
        extern i32 motor_dir[];
        motor_dir[0] = strtol(argv[2], nullptr, 0);
        motor_dir[1] = strtol(argv[3], nullptr, 0);
    } else if (strlen("get") == argv1len && strncmp("get", argv[1], argv1len) == 0) {
        console.printf("p = %.2f i = %.2f d = %.2f\n", kp, ki, kd);
    }
    return 0;
}

int move_cmd_handler(const u8 argc, char **argv) {
    if (argc < 2) { console.printf("too few args\n"); return 1; }
    const u8 argv1len = strlen(argv[1]);
    f32 val;

    extern volatile f32 target_speed_rpm[2];
    static f32 duration = 1.95;

    if (strlen("left") == argv1len && strncmp("left", argv[1], argv1len) == 0) {
        if (argc < 3) { console.printf("too few args\n"); return 1; }
        val = strtof(argv[2], nullptr);
        target_speed_rpm[0] = -val;
        target_speed_rpm[1] = val;
        vTaskDelay(duration * configTICK_RATE_HZ);
        target_speed_rpm[0] = 0;
        target_speed_rpm[1] = 0;
    } else if (strlen("right") == argv1len && strncmp("right", argv[1], argv1len) == 0) {
        if (argc < 3) { console.printf("too few args\n"); return 1; }
        val = strtof(argv[2], nullptr);
        target_speed_rpm[0] = val;
        target_speed_rpm[1] = -val;
        vTaskDelay(duration * configTICK_RATE_HZ);
        target_speed_rpm[0] = 0;
        target_speed_rpm[1] = 0;
    } else if (strlen("front") == argv1len && strncmp("front", argv[1], argv1len) == 0) {
        if (argc < 3) { console.printf("too few args\n"); return 1; }
        val = strtof(argv[2], nullptr);
        target_speed_rpm[0] = val;
        target_speed_rpm[1] = val;
        vTaskDelay(duration * configTICK_RATE_HZ);
        target_speed_rpm[0] = 0;
        target_speed_rpm[1] = 0;
    } else if (strlen("back") == argv1len && strncmp("back", argv[1], argv1len) == 0) {
        if (argc < 3) { console.printf("too few args\n"); return 1; }
        val = strtof(argv[2], nullptr);
        target_speed_rpm[0] = -val;
        target_speed_rpm[1] = -val;
        vTaskDelay(duration * configTICK_RATE_HZ);
        target_speed_rpm[0] = 0;
        target_speed_rpm[1] = 0;
    } else if (strlen("settime") == argv1len && strncmp("settime", argv[1], argv1len) == 0) {
        if (argc < 3) { console.printf("too few args\n"); return 1; }
        duration = strtof(argv[2], nullptr);
    }
    return 0;
}

int demo_cmd_handler(const u8 argc, char **argv) {
    if (argc < 2) { console.printf("too few args\n"); return 1; }
    const u8 argv1len = strlen(argv[1]);

    if (strlen("set90") == argv1len && strncmp("set90", argv[1], argv1len) == 0) {
        if (argc < 3) { console.printf("too few args\n"); return 1; }
        extern f32 durationFor90;
        durationFor90 = strtof(argv[2], nullptr);
    } else if (strlen("setrpm") == argv1len && strncmp("setrpm", argv[1], argv1len) == 0) {
        if (argc < 4) { console.printf("too few args\n"); return 1; }
        extern f32 demo_rpm_turn, demo_rpm_straight;
        demo_rpm_turn = strtof(argv[2], nullptr);
        demo_rpm_straight = strtof(argv[3], nullptr);
    } else if (strlen("setsvd") == argv1len && strncmp("setsvd", argv[1], argv1len) == 0) {
        if (argc < 3) { console.printf("too few args\n"); return 1; }
        extern u32 demo_servo_delay;
        demo_servo_delay = strtoul(argv[2], nullptr, 0);
    } else if (strlen("setthr") == argv1len && strncmp("setthr", argv[1], argv1len) == 0) {
        if (argc < 3) { console.printf("too few args\n"); return 1; }
        extern i32 demo_rssi_threshould;
        demo_rssi_threshould = strtol(argv[2], nullptr, 0);
    }
    return 0;
}

int rssi_cmd_handler(const u8 argc, char **argv) {
    extern i16 rssi[];
    console.printf("%3d %3d\n", rssi[0], rssi[1]);
    return 0;
}

xHAL::ShellCommand cmds[] = {
    {"hello", [](const u8 argc, char **argv)->int { console.printf("hello this is shell\n"); return 0; } },
    {"rx_overflow", [](const u8 argc, char **argv)->int { console.printf("error: rx_overflow\n"); return 1; } },
    {"i2c1", &i2c1_cmd_handler},
    {"task", &taskControl_cmd_handler},
    {"setpwm", &setpwm_cmd_handler},
    {"blink", &blink_cmd_handler},
    {"mem", &mem_cmd_handler},
    {"m4", &m4_cmd_handler},
    {"move", &move_cmd_handler},
    {"demo", &demo_cmd_handler},
    {"rssi", &rssi_cmd_handler},
    {nullptr, nullptr}
};