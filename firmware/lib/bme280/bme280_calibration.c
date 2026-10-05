#include "bme280_calibration.h"

#include <stddef.h>

#include "bme280.h"

// Registers (BME280 datasheet section 4.2.2)
#define REG_CALIB_A 0x88  // 26 bytes, up to 0xA1: temperature, pressure and the first humidity value
#define REG_CALIB_B 0xE1  // 7 bytes, up to 0xE7: the rest of the humidity values

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

    if (!bme280_read_registers(REG_CALIB_A, a, sizeof(a))) {
        return false;
    }
    if (!bme280_read_registers(REG_CALIB_B, b, sizeof(b))) {
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
