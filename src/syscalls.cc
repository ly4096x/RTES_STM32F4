extern "C" {
#include <errno.h>
#include <malloc.h>
#include <sys/unistd.h>
#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>
}
#include "board_common.h"
#include <xHAL/USART>
#include <xHAL/Mutex>

#if 0
static xHAL::Mutex print_console_mutex;
extern "C"
int _write_working_but_disabled(int file, char *data, int len) {
    if (file != STDOUT_FILENO && file != STDERR_FILENO) {
        errno = EBADF;
        return -1;
    }
    if (len == 0) return 0;

    extern xHAL::USART console;
    print_console_mutex.lock();
    u32 wrote_len = console.write((u8 *)data, len);
    print_console_mutex.unlock();
    if (wrote_len != len) FailAndInfiniteLoop();

    return len;
}
#endif

extern "C"
void *malloc(size_t size) {
    void *ptr = nullptr;
    if (size > 0)
        ptr = pvPortMalloc(size);
    return ptr;
}

extern "C"
void free(void *ptr) {
    if (ptr)
        vPortFree(ptr);
}

void *operator new(size_t size) { return malloc(size); }

void *operator new[](size_t size) { return malloc(size); }

void operator delete(void *p) { free(p); }

void operator delete[](void *p) { free(p); }