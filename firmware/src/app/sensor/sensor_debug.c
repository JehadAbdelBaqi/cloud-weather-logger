#include "sensor_debug.h"

#include <stddef.h>

#include "debug.h"

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

void sensor_debug_show_chip(uint8_t address, uint8_t chip_id)
{
    debug_print("BME280 at address ");
    debug_print_hex(address);
    debug_print(", chip ID ");
    debug_print_hex(chip_id);
    debug_print("\r\n");
}

void sensor_debug_show_calibration(const bme280_calibration_t *calibration)
{
    const bme280_calibration_t *c = calibration;

    const int32_t t[] = { c->t1, c->t2, c->t3 };
    const int32_t p[] = { c->p1, c->p2, c->p3, c->p4, c->p5, c->p6, c->p7, c->p8, c->p9 };
    const int32_t h[] = { c->h1, c->h2, c->h3, c->h4, c->h5, c->h6 };

    show_values("calibration T:", t, sizeof(t) / sizeof(t[0]));
    show_values("calibration P:", p, sizeof(p) / sizeof(p[0]));
    show_values("calibration H:", h, sizeof(h) / sizeof(h[0]));
}

void sensor_debug_show_raw(const bme280_raw_t *raw)
{
    debug_print("raw: temperature ");
    debug_print_number(raw->temperature);
    debug_print(", pressure ");
    debug_print_number(raw->pressure);
    debug_print(", humidity ");
    debug_print_number(raw->humidity);
    debug_print("\r\n");
}

void sensor_debug_show_reading(const reading_t *reading)
{
    debug_print("compensated: temperature ");
    debug_print_signed(reading->temperature);
    debug_print(" hundredths of a degree C, pressure ");
    debug_print_number(reading->pressure);
    debug_print(" Pa, humidity ");
    debug_print_number(reading->humidity);
    debug_print(" hundredths of a percent\r\n");
}
