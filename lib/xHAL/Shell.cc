#include <xHAL/Shell>
#include <xHAL/USART>
#include <FreeRTOS.h>
#include <task.h>
#include <cstring>
#include <cinttypes>

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
    console.write((u8*)"\n", 1);
    console.setEcho(true);
    while (1) {
        console.printf(">>> ");
        u32 len = console.readline(line, sizeof(line), -1);
        handleCommand(line);
        console.printf("[%s] STACK_UNUSED = %10" PRIu32 "\n",
                pcTaskGetName(xTaskGetCurrentTaskHandle()), uxTaskGetStackHighWaterMark(xTaskGetCurrentTaskHandle()));
    }
}

int Shell::handleCommand(char *line) {
    if (!*line) return 0;

    char *argv0_begin = line;
    while (*argv0_begin == ' ' || *argv0_begin == '\n' || *argv0_begin == '\t') // remove extra spaces
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