#include "bme280_driver.h"

#include <stddef.h>

#include "stm32f4xx_hal.h"

#include "time_interval.h"

// The two addresses the sensor can have, set by its address pin (BME280 datasheet, I2C interface)
#define ADDRESS_LOW  0x76
#define ADDRESS_HIGH 0x77

// Registers (BME280 datasheet section 5.4)
#define REG_CALIB_A   0x88  // BME280_CALIBRATION_A_LENGTH bytes
#define REG_CHIP_ID   0xD0
#define REG_CALIB_B   0xE1  // BME280_CALIBRATION_B_LENGTH bytes
#define REG_CTRL_HUM  0xF2
#define REG_STATUS    0xF3
#define REG_CTRL_MEAS 0xF4
#define REG_DATA      0xF7  // BME280_RAW_LENGTH bytes

// ctrl_hum: humidity measured once per measurement (oversampling x1)
#define CTRL_HUM_X1 0x01

// ctrl_meas: temperature x1 (bits 7-5), pressure x1 (bits 4-2), forced mode (bits 1-0):
// the sensor takes one measurement and goes back to sleep
#define CTRL_MEAS_X1_FORCED ((0x01 << 5) | (0x01 << 2) | 0x01)

// status: set while a measurement is running
#define STATUS_MEASURING (1 << 3)

// Longest wait for one measurement, in milliseconds
#define MEASURE_TIMEOUT_MS 50

static i2c_t sensor_bus;
static uint8_t sensor_address = 0;

bool bme280_init(const i2c_config_t *config)
{
    if (!i2c_init(&sensor_bus, config)) {
        return false;
    }

    if (i2c_device_present(&sensor_bus, ADDRESS_LOW)) {
        sensor_address = ADDRESS_LOW;
        return true;
    }
    if (i2c_device_present(&sensor_bus, ADDRESS_HIGH)) {
        sensor_address = ADDRESS_HIGH;
        return true;
    }
    return false;
}

uint8_t bme280_address(void)
{
    return sensor_address;
}

bool bme280_read_chip_id(uint8_t *chip_id)
{
    return i2c_read(&sensor_bus, sensor_address, REG_CHIP_ID, chip_id, 1);
}

/**
 * @brief Reads a run of the sensor's registers.
 *
 * @param reg    The first register
 * @param data   Filled in with the bytes read
 * @param length How many bytes
 * @return true if they were read
 */
static bool read_registers(uint8_t reg, uint8_t *data, size_t length)
{
    return i2c_read(&sensor_bus, sensor_address, reg, data, length);
}

/**
 * @brief Writes one byte to one of the sensor's registers.
 *
 * @param reg   The register
 * @param value The byte
 * @return true if it was written
 */
static bool write_register(uint8_t reg, uint8_t value)
{
    return i2c_write(&sensor_bus, sensor_address, reg, &value, 1);
}

/**
 * @brief Asks the sensor for one measurement of humidity, temperature and pressure.
 *
 * The humidity setting only takes effect once ctrl_meas is written, so it is written first.
 *
 * @return true if both settings were written
 */
static bool start_measurement(void)
{
    if (!write_register(REG_CTRL_HUM, CTRL_HUM_X1)) {
        return false;
    }
    return write_register(REG_CTRL_MEAS, CTRL_MEAS_X1_FORCED);
}

/**
 * @brief Waits until the sensor has finished its measurement.
 *
 * @return true once the sensor says it is done, false if it does not answer or takes too long
 */
static bool wait_measurement_done(void)
{
    uint32_t started_ms = HAL_GetTick();
    uint8_t status = STATUS_MEASURING;

    HAL_Delay(2);  // the sensor needs a moment before it shows that it is measuring

    while (status & STATUS_MEASURING) {
        if (time_interval_passed(HAL_GetTick(), started_ms, MEASURE_TIMEOUT_MS)) {
            return false;
        }
        if (!i2c_read(&sensor_bus, sensor_address, REG_STATUS, &status, 1)) {
            return false;
        }
    }
    return true;
}

bool bme280_read_raw(bme280_raw_t *raw)
{
    uint8_t data[BME280_RAW_LENGTH];

    if (!start_measurement()) {
        return false;
    }
    if (!wait_measurement_done()) {
        return false;
    }
    if (!i2c_read(&sensor_bus, sensor_address, REG_DATA, data, sizeof(data))) {
        return false;
    }

    bme280_raw_from_bytes(data, raw);
    return true;
}

bool bme280_read_calibration(bme280_calibration_t *calibration)
{
    uint8_t a[BME280_CALIBRATION_A_LENGTH];
    uint8_t b[BME280_CALIBRATION_B_LENGTH];

    if (!read_registers(REG_CALIB_A, a, sizeof(a))) {
        return false;
    }
    if (!read_registers(REG_CALIB_B, b, sizeof(b))) {
        return false;
    }

    bme280_calibration_from_bytes(a, b, calibration);
    return true;
}
