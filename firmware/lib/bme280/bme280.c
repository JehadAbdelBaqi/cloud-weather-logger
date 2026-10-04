#include "bme280.h"

#include <stddef.h>

#include "stm32f4xx_hal.h"

#include "board.h"
#include "i2c.h"

// The two addresses the sensor can have, set by its address pin (BME280 datasheet, I2C interface)
#define ADDRESS_LOW  0x76
#define ADDRESS_HIGH 0x77

// Registers (BME280 datasheet section 5.4)
#define REG_CALIB_A   0x88  // 26 bytes, up to 0xA1: temperature, pressure and the first humidity value
#define REG_CHIP_ID   0xD0
#define REG_CALIB_B   0xE1  // 7 bytes, up to 0xE7: the rest of the humidity values
#define REG_CTRL_HUM  0xF2
#define REG_STATUS    0xF3
#define REG_CTRL_MEAS 0xF4
#define REG_DATA      0xF7  // 8 bytes: pressure (3), temperature (3), humidity (2)

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

bool bme280_init(void)
{
    const i2c_config_t config = {
        .instance = SENSOR_I2C,
        .port = SENSOR_PORT,
        .scl_pin = SENSOR_SCL_PIN,
        .sda_pin = SENSOR_SDA_PIN,
        .pin_af = SENSOR_PIN_AF,
        .speed_hz = SENSOR_I2C_SPEED_HZ,
    };
    if (!i2c_init(&sensor_bus, &config)) {
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
 * @brief Joins two bytes into an unsigned 16-bit number, low byte first.
 *
 * @param data The bytes
 * @param at   Where the low byte is
 * @return The number
 */
static uint16_t u16_at(const uint8_t *data, size_t at)
{
    return (uint16_t)(data[at] | (data[at + 1] << 8));
}

/**
 * @brief Joins two bytes into a signed 16-bit number, low byte first.
 *
 * @param data The bytes
 * @param at   Where the low byte is
 * @return The number
 */
static int16_t s16_at(const uint8_t *data, size_t at)
{
    return (int16_t)u16_at(data, at);
}

bool bme280_read_calibration(bme280_calibration_t *calibration)
{
    uint8_t a[26];  // registers 0x88 to 0xA1
    uint8_t b[7];   // registers 0xE1 to 0xE7

    if (!i2c_read(&sensor_bus, sensor_address, REG_CALIB_A, a, sizeof(a))) {
        return false;
    }
    if (!i2c_read(&sensor_bus, sensor_address, REG_CALIB_B, b, sizeof(b))) {
        return false;
    }

    calibration->t1 = u16_at(a, 0);
    calibration->t2 = s16_at(a, 2);
    calibration->t3 = s16_at(a, 4);

    calibration->p1 = u16_at(a, 6);
    calibration->p2 = s16_at(a, 8);
    calibration->p3 = s16_at(a, 10);
    calibration->p4 = s16_at(a, 12);
    calibration->p5 = s16_at(a, 14);
    calibration->p6 = s16_at(a, 16);
    calibration->p7 = s16_at(a, 18);
    calibration->p8 = s16_at(a, 20);
    calibration->p9 = s16_at(a, 22);

    calibration->h1 = a[25];  // register 0xA1
    calibration->h2 = s16_at(b, 0);
    calibration->h3 = b[2];
    // h4 and h5 are 12 bits each and share the byte at 0xE5: its low half ends h4, its high half starts h5
    calibration->h4 = (int16_t)(((int8_t)b[3] * 16) | (b[4] & 0x0F));
    calibration->h5 = (int16_t)(((int8_t)b[5] * 16) | (b[4] >> 4));
    calibration->h6 = (int8_t)b[6];
    return true;
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
        if (HAL_GetTick() - started_ms > MEASURE_TIMEOUT_MS) {
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
    uint8_t data[8];

    if (!start_measurement()) {
        return false;
    }
    if (!wait_measurement_done()) {
        return false;
    }
    if (!i2c_read(&sensor_bus, sensor_address, REG_DATA, data, sizeof(data))) {
        return false;
    }

    // Pressure and temperature are 20 bits each: 8 + 8 + the top 4 bits of a third byte
    raw->pressure = ((uint32_t)data[0] << 12) | ((uint32_t)data[1] << 4) | (data[2] >> 4);
    raw->temperature = ((uint32_t)data[3] << 12) | ((uint32_t)data[4] << 4) | (data[5] >> 4);
    raw->humidity = (uint16_t)((data[6] << 8) | data[7]);
    return true;
}
