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