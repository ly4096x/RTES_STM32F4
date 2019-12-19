#include "board_common.h"
#include <xHAL/I2C>
#include <xHAL/USART>
#include <xHAL/Shell>
#include <cctype>
#include <cstring>
#include <cstdlib>

extern "C" {
#include "tim.h"
}

extern xHAL::USART console;
extern xHAL::I2C xI2C1;

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

    extern f32 pid_param[4], target_speed_rpm;
    f32 &kp = pid_param[0], &ki = pid_param[1], &kd = pid_param[2], &dir = pid_param[3];

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
        if (argc < 3) { console.printf("too few args\n"); return 1; }
        val = strtof(argv[2], nullptr);
        target_speed_rpm = val;
    } else if (strlen("dir") == argv1len && strncmp("dir", argv[1], argv1len) == 0) {
        if (argc < 3) { console.printf("too few args\n"); return 1; }
        val = strtof(argv[2], nullptr);
        dir = val;
    } else if (strlen("get") == argv1len && strncmp("get", argv[1], argv1len) == 0) {
        console.printf("p = %.2f i = %.2f d = %.2f\n", kp, ki, kd);
    }
    return 0;
}

xHAL::ShellCommand cmds[] = {
    {"hello", [](const u8 argc, char **argv)->int { console.printf("hello this is shell\n"); return 0; } },
    {"rx_overflow", [](const u8 argc, char **argv)->int { console.printf("error: rx_overflow\n"); return 1; } },
    {"i2c1", &i2c1_cmd_handler},
    {"task", &taskControl_cmd_handler},
    {"setpwm", &setpwm_cmd_handler},
    {"mem", &mem_cmd_handler},
    {"m4", &m4_cmd_handler},
    {nullptr, nullptr}
};