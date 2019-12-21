#include <xHAL/USART>
#include <FreeRTOS.h>
#include <cinttypes>
#include <main.h>
#include <stm32f4xx_ll_gpio.h>

extern xHAL::USART console, xUART4, xUART5;

/*
rssi : RSSI value received from each ESP8266
(Only using 1 of them since the proof of concept failed for dual antenna navigation)
*/
i16 rssi[2] = {0};

void rssi_receiver_1_thread(void *param) {
    i32 rssi_new;
    constexpr i8 rssi_filter_window = 8;
    i16 rssi_filter_buf[rssi_filter_window] = {0}, rssi_filter_sum = 0, *rssi_filter_buf_ptr = rssi_filter_buf;
    while (true) {
        u8 zero_counter = 0;
        while (true) {
            auto c = xUART4.getChar(DEFAULT_TIMEOUT, false, true);
            if (c == 0) ++zero_counter;
            else zero_counter = 0;
            if (zero_counter == 4) break;
        }

        u8 *rssi_new_byte_ptr = (u8*)&rssi_new;
        for (u8 i = 0; i != 4; ++i)
            *rssi_new_byte_ptr++ = xUART4.getChar(DEFAULT_TIMEOUT, false, true);

        rssi_filter_sum -= *rssi_filter_buf_ptr;
        rssi_filter_sum += rssi_new;
        *rssi_filter_buf_ptr++ = rssi_new;
        if (rssi_filter_buf_ptr - rssi_filter_buf == rssi_filter_window)
            rssi_filter_buf_ptr = rssi_filter_buf;

        rssi[0] = rssi_filter_sum / rssi_filter_window;
        //console.printf("[rssi1] %3d\n", rssi[0]);
    }
}

void rssi_receiver_2_thread(void *param) {
    i32 rssi_new;
    constexpr i8 rssi_filter_window = 8;
    i16 rssi_filter_buf[rssi_filter_window] = {0}, rssi_filter_sum = 0, *rssi_filter_buf_ptr = rssi_filter_buf;
    while (true) {
        u8 zero_counter = 0;
        while (true) {
            auto c = xUART5.getChar(DEFAULT_TIMEOUT, false, true);
            if (c == 0) ++zero_counter;
            else zero_counter = 0;
            if (zero_counter == 4) break;
        }

        u8 *rssi_new_byte_ptr = (u8*)&rssi_new;
        for (u8 i = 0; i != 4; ++i)
            *rssi_new_byte_ptr++ = xUART5.getChar(DEFAULT_TIMEOUT, false, true);

        if (rssi_new >= 0) continue;

        rssi_filter_sum -= *rssi_filter_buf_ptr;
        rssi_filter_sum += rssi_new;
        *rssi_filter_buf_ptr++ = rssi_new;
        if (rssi_filter_buf_ptr - rssi_filter_buf == rssi_filter_window)
            rssi_filter_buf_ptr = rssi_filter_buf;

        rssi[1] = rssi_filter_sum / rssi_filter_window;
        //console.printf("[rssi1] %3d [rssi2] %3d [diff] %3d\n", rssi[0], rssi[1], rssi[0] - rssi[1]);
    }
}