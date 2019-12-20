#include <xHAL/SerialDevice>
#include <xHAL/ThreadUtils>
#include <cstring>

namespace xHAL {

SerialDevice::SerialDevice(const TaskNotificationId _txNotifyId, const TaskNotificationId _rxNotifyId) :
        txMutex(true),
        txNotifyId(_txNotifyId),
        rxNotifyId(_rxNotifyId),
        rxbuf_begin(rxbuf),
        rxbuf_end(rxbuf),
        rxEcho(0)
{
    txbuf_end = txbuf;
}

u32 SerialDevice::writeCharToTxBuffer(char c, bool flush, u32 deadline) {
    AutoLock lock(txMutex, deadline);
    if (!lock.successful()) return 0;

    if (!(c == 0 && flush))
        *txbuf_end++ = c;
    if (txbuf_end == txbuf + sizeof(txbuf) || c == '\n' || flush) {
        write((u8 *)txbuf, txbuf_end - txbuf, deadline);
        txbuf_end = txbuf;
    }
    return 1;
}

u32 SerialDevice::writeToTxBuffer(char *data, u32 len, u32 deadline) {
    while (len--)
        writeCharToTxBuffer(*data++, false, deadline);
    return len;
}

u32 SerialDevice::printf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    AutoLock lock(txMutex);
    if (!lock.successful()) return 0;

    u32 len = _vprintf(fmt, args);
    writeCharToTxBuffer(0, 1);

    va_end(args);
    return len;
}

char SerialDevice::getChar(u32 timeout, bool doNotEchoIfBackspace, bool waitForever) {
    char ret;
    ret = peekChar(getDeadline(timeout), waitForever);
    if (rxEcho) {
        u32 echoDeadline = getDeadline(timeout);
        if (terminal_mode && ret == '\b') {
            if (!doNotEchoIfBackspace) {
                writeToTxBuffer("\b \b", 3, echoDeadline);
                writeCharToTxBuffer(0, true, echoDeadline); // flush
            }
        } else
            writeCharToTxBuffer(ret, true, echoDeadline);
    }
    ++rxbuf_begin;
    if (rxbuf_begin == rxbuf + sizeof(rxbuf))
        rxbuf_begin = rxbuf;
    return ret;
}
char SerialDevice::peekChar(u32 deadline, bool waitForever) {
    if (rxbuf_begin == rxbuf_end) {
        rxCaller = xTaskGetCurrentTaskHandle();
        waitForNotification(rxNotifyId, deadline, waitForever);
    }
    return *rxbuf_begin;
}

u32 SerialDevice::readline(char *_dst, const u32 maxLength, u32 deadline, bool waitForever) {
    AutoLock lock(rxMutex, deadline);
    if (!lock.successful()) return 0;

    u32 echoTimeout = getTimeout(deadline);

    const char *dst = _dst;
    char *dst_end = _dst;
    while ((u32)(dst_end - dst) != maxLength - 1) {
        *dst_end = getChar(echoTimeout, dst_end == dst, waitForever);
        if (*dst_end == '\b') {
            if (dst_end != dst)
                --dst_end;
            continue;
        }
        if (*dst_end == '\n') break;
        ++dst_end;
    }

    int retlen = dst_end - _dst;
    if (*dst_end != '\n') {
        while (getChar(echoTimeout, true, waitForever) != '\n');
        const char *msg = "rx_overflow";
        strcpy(_dst, msg);
        retlen = strlen(msg);
    }
    
    *dst_end = 0;
    return retlen;
}

}