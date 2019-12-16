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
{}

static inline char *next_arg(char *&arg_start) {
    while (isspace(*arg_start)) // remove extra spaces
        ++arg_start;
    char *arg_end = arg_start;
    while (*arg_end && *arg_end != ' ')
        ++arg_end;
    return arg_end;
}

const u8 Shell::parse_args(char *arg_start) {
    char *arg_end;
    char **argv_ptr = argv_list;
    while (true) {
        arg_end = next_arg(arg_start);
        if (arg_start == arg_end) break;
        *arg_end = '\0';
        *argv_ptr++ = arg_start;
    }
    return argv_ptr - argv_list;
}

void Shell::run() {
    char line[64];
    console.setEcho(true);
    u32 timerValue;
    while (1) {
        console.printf("\n>>> ");
        u32 len = console.readline(line, sizeof(line));
        if (len && line[len - 1] == '\0') { // maformed string
            timerValue = get_cycle_counter_value();
            handleCommand(line, len);
            console.printf("\nexec time: %.3fms\n[%s] STACK_UNUSED = %10" PRIu32,
                (get_cycle_counter_value() - timerValue) / (SystemCoreClock / 1000.f),
                pcTaskGetName(xTaskGetCurrentTaskHandle()),
                uxTaskGetStackHighWaterMark(xTaskGetCurrentTaskHandle())
            );
        }
    }
}

int Shell::handleCommand(char *line, const u32 lineLen) {
    const u8 argc = parse_args(line);
    if (!argc) return 0;
    
    ShellCommand *cmd = commandsList;
    auto &argv = argv_list;

    do {
        const u8 cmd_len = strlen(cmd->cmd), arg_len = strlen(argv[0]);
        if (cmd_len == arg_len && strncmp(cmd->cmd, argv[0], cmd_len) == 0)
            return cmd->func(argc, argv);
    } while ((++cmd)->cmd);
    
    console.printf("shell: No such command \"%s\"\n", argv[0]);
    return -1;
}

}