#include <stddef.h>

#include "stm32f4xx_hal.h"

#include "bme280.h"
#include "clock.h"
#include "config.h"
#include "debug.h"
#include "handshake.h"
#include "led.h"
#include "link.h"

/**
 * @brief SysTick interrupt, fires every 1 ms. Advances the HAL's millisecond counter.
 */
void SysTick_Handler(void)
{
    HAL_IncTick();
}

/**
 * @brief Stops the program after a setup step has failed. Never returns.
 */
static void error_halt(void)
{
    __disable_irq();
    while (1) {
    }
}

/**
 * @brief Shows the link to the bridge on the user LED: a fast blink while the connection is being
 *        made, a slow blink once it is.
 */
static void show_link_state(void)
{
    static uint32_t last_toggle_ms = 0;
    uint32_t blink_ms = handshake_connected() ? LED_CONNECTED_BLINK_MS : LED_CONNECTING_BLINK_MS;

    if (HAL_GetTick() - last_toggle_ms < blink_ms) {
        return;
    }

    last_toggle_ms = HAL_GetTick();
    led_toggle();
}

/**
 * @brief Deals with a line that has arrived from the bridge, if one is complete.
 *
 * A handshake line goes to the handshake; any other line is written to the debug output.
 */
static void handle_received_line(void)
{
    const char *line = link_read_line();
    if (line == NULL) {
        return;
    }

    if (handshake_handle_line(line)) {
        return;
    }

    debug_print("down: ");
    debug_print(line);
    debug_print("\r\n");
}

/**
 * @brief Finds the BME280 and writes its address and chip ID to the debug output.
 */
static void show_sensor(void)
{
    uint8_t chip_id = 0;

    if (!bme280_init()) {
        debug_print("BME280 not found\r\n");
        return;
    }
    if (!bme280_read_chip_id(&chip_id)) {
        debug_print("BME280 found, but its chip ID could not be read\r\n");
        return;
    }

    debug_print("BME280 at address ");
    debug_print_hex(bme280_address());
    debug_print(", chip ID ");
    debug_print_hex(chip_id);
    debug_print("\r\n");
}

/**
 * @brief Writes a list of numbers to the debug output on one line, after a label.
 *
 * @param label  Text to start the line with
 * @param values The numbers
 * @param count  How many numbers
 */
static void show_values(const char *label, const int32_t *values, size_t count)
{
    debug_print(label);
    for (size_t i = 0; i < count; i++) {
        debug_print(" ");
        debug_print_signed(values[i]);
    }
    debug_print("\r\n");
}

/**
 * @brief Reads the sensor's calibration values and writes them to the debug output.
 */
static void show_calibration(void)
{
    bme280_calibration_t c;

    if (!bme280_read_calibration(&c)) {
        debug_print("calibration: could not be read\r\n");
        return;
    }

    const int32_t t[] = { c.t1, c.t2, c.t3 };
    const int32_t p[] = { c.p1, c.p2, c.p3, c.p4, c.p5, c.p6, c.p7, c.p8, c.p9 };
    const int32_t h[] = { c.h1, c.h2, c.h3, c.h4, c.h5, c.h6 };

    show_values("calibration T:", t, sizeof(t) / sizeof(t[0]));
    show_values("calibration P:", p, sizeof(p) / sizeof(p[0]));
    show_values("calibration H:", h, sizeof(h) / sizeof(h[0]));
}

/**
 * @brief Takes a measurement and writes its raw values to the debug output, once the interval
 *        has passed.
 *
 * Does not wait when it is not time yet. Taking the measurement itself takes about 10 ms.
 */
static void show_raw_reading_when_due(void)
{
    static uint32_t last_read_ms = 0;
    bme280_raw_t raw;

    if (HAL_GetTick() - last_read_ms < SENSOR_READ_INTERVAL_MS) {
        return;
    }
    last_read_ms = HAL_GetTick();

    if (!bme280_read_raw(&raw)) {
        debug_print("raw: the sensor could not be read\r\n");
        return;
    }

    debug_print("raw: temperature ");
    debug_print_number(raw.temperature);
    debug_print(", pressure ");
    debug_print_number(raw.pressure);
    debug_print(", humidity ");
    debug_print_number(raw.humidity);
    debug_print("\r\n");
}

/**
 * @brief Entry point, called by the startup code after reset.
 */
int main(void)
{
    HAL_Init();
    if (!clock_init()) {
        error_halt();
    }
    led_init();
    if (!debug_init()) {
        error_halt();
    }
    if (!link_init()) {
        error_halt();
    }

    debug_print("weather logger started\r\n");
    show_sensor();
    show_calibration();
    handshake_init();

    while (1) {
        handle_received_line();
        handshake_poll();
        show_link_state();
        show_raw_reading_when_due();
    }
}
