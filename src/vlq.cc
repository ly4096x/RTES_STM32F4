#include <vl53l1x-arduino/VL53L1X.h>
#include <xHAL/USART>

extern xHAL::USART console;
VL53L1X *sensor_ptr;

// VL53L1X test thread
void vlq_thread(void *param) {
    vTaskDelay(1 * configTICK_RATE_HZ);

    // initialize VL53L1X
    VL53L1X sensor;
    sensor_ptr = &sensor;
    sensor.setTimeout(500);
    if (!sensor.init()) {
        console.printf("[vlq] Failed to detect and initialize sensor!\n");
        while (1)
            ;
    }

    // Use long distance mode and allow up to 50000 us (50 ms) for a measurement.
    // You can change these settings to adjust the performance of the sensor, but
    // the minimum timing budget is 20 ms for short distance mode and 33 ms for
    // medium and long distance modes. See the VL53L1X datasheet for more
    // information on range and timing limits.
    sensor.setDistanceMode(VL53L1X::Short);
    sensor.setMeasurementTimingBudget(50000);
    sensor.startContinuous(50);

    vTaskSuspend(nullptr);

    // Start continuous readings at a rate of one measurement every 50 ms (the
    // inter-measurement period). This period should be at least as long as the
    // timing budget.
    while (true) {
        console.printf("[vlq] %7d %4d\n", millis(), sensor.read());
        if (sensor.timeoutOccurred()) {
            console.printf("[vlq] TIMEOUT\n");
        }
        vTaskDelay(1);
    }
}