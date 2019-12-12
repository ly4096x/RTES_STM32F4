#include <xHAL/Shell>
#include <xHAL/USART>
#include <FreeRTOS.h>
#include <task.h>
#include <cstring>
#include <cinttypes>
#include <cctype>

extern xHAL::USART console;

namespace xHAL {

Shell::Shell(ShellCommand *cmds) :
        commandsList(cmds)
{
    auto *cmd = commandsList;
    while (cmd->cmd) {
        cmd->len = strlen(cmd->cmd);
        ++cmd;
    }
}

void Shell::run() {
    char line[64];
    console.setEcho(true);
    u32 timerValue;
    while (1) {
        console.printf("\n>>> ");
        u32 len = console.readline(line, sizeof(line));
        if (len && line[len - 1] != 0x03) { // ctrl-c
            timerValue = get_cycle_counter_value();
            handleCommand(line, len);
            console.printf("\nexec time: %.3fms\n", (get_cycle_counter_value() - timerValue) / (SystemCoreClock / 1000.f));
        }
        console.printf("[%s] STACK_UNUSED = %10" PRIu32,
                pcTaskGetName(xTaskGetCurrentTaskHandle()), uxTaskGetStackHighWaterMark(xTaskGetCurrentTaskHandle()));
    }
}

int Shell::handleCommand(char *line, const u32 lineLen) {
    if (!*line) return 0;

    char *argv0_begin = line;
    while (isspace(*argv0_begin)) // remove extra spaces
        ++argv0_begin;
    char *argv0_end = argv0_begin;
    while (*argv0_end && *argv0_end != ' ')
        ++argv0_end;
    
    ShellCommand *cmd = commandsList;
    while (cmd->cmd && (cmd->len != argv0_end - argv0_begin || strncmp(cmd->cmd, argv0_begin, cmd->len)))
        ++cmd;
    int ret = -1;
    if (cmd->cmd) {
        char *argv1_begin = argv0_end;
        while (*argv1_begin == ' ') // remove extra spaces
            ++argv1_begin;
        ret = cmd->func(argv1_begin);
    } else {
        console.printf("sh: No such command \"%s\"\n", argv0_begin);
    }
    return ret;
}

}