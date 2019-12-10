#include <xHAL/SerialDevice>
#include <cstring>

namespace xHAL {

SerialDevice::SerialDevice(const TaskNotificationIds _txNotifyId, const TaskNotificationIds _rxNotifyId) :
        txMutex(true),
        txNotifyId(_txNotifyId),
        rxNotifyId(_rxNotifyId),
        rxbuf_begin(rxbuf),
        rxbuf_end(rxbuf),
        rxEcho(0)
{}

u32 SerialDevice::writeCharToBulk(char c, bool flush, u32 timeout) {
    static char txbuf[64], *txbuf_end = txbuf;
    if (!(c == 0 && flush))
        *txbuf_end++ = c;
    if (txbuf_end == txbuf + sizeof(txbuf) || c == '\n' || flush) {
        write((u8 *)txbuf, txbuf_end - txbuf, timeout);
        txbuf_end = txbuf;
    }
    return 1;
}

u32 SerialDevice::writeToBulk(char *data, u32 len, u32 timeout) {
    while (len--)
        writeCharToBulk(*data++, timeout);
    return len;
}

u32 SerialDevice::printf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    if (!txMutex.lock()) return 0;
    u32 len = _vprintf(fmt, args);
    writeCharToBulk(0, 1);
    txMutex.unlock();

    va_end(args);
    return len;
}

char SerialDevice::getChar(u32 timeout) {
    char ret;
    ret = peekChar(timeout);
    if (rxEcho) {
        writeCharToBulk(ret, true, timeout);
    }
    ++rxbuf_begin;
    if (rxbuf_begin == rxbuf + sizeof(rxbuf))
        rxbuf_begin = rxbuf;
    return ret;
}
char SerialDevice::peekChar(u32 timeout) {
    if (rxbuf_begin == rxbuf_end) {
        rxCaller = xTaskGetCurrentTaskHandle();
        u32 notifiedValue = INVALID_NOTIFY_VALUE;
        while (notifiedValue != rxNotifyId) {
            if (!xTaskNotifyWait(0, 0, &notifiedValue, timeout))
                FailAndInfiniteLoop();
        }
    }
    return *rxbuf_begin;
}

u32 SerialDevice::readline(char *_dst, const u32 maxLength, u32 timeout) {
    if (!rxMutex.lock(timeout)) return 0;

    const char *dst = _dst;
    char *dst_end = _dst;
    while (dst_end - dst != maxLength - 1) {
        *dst_end = getChar(timeout);
        if (*dst_end == 0x0c && dst_end > dst) {
            --dst_end;
            continue;
        }
        if (*dst_end == '\n') break;
        ++dst_end;
    }

    int retlen = dst_end - _dst;
    if (*dst_end != '\n') {
        while (getChar(timeout) != '\n');
        const char *msg = "rx_overflow";
        strcpy(_dst, msg);
        retlen = strlen(msg);
    }
    rxMutex.unlock();
    
    *dst_end = 0;
    return retlen;
}

}