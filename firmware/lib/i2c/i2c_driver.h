#ifndef I2C_DRIVER_H
#define I2C_DRIVER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "stm32f4xx_hal.h"

/** One I2C bus, once i2c_init() has set it up. */
typedef I2C_HandleTypeDef i2c_t;

/** Which I2C bus to set up, the pins it uses and its speed. */
typedef struct {
    I2C_TypeDef *instance;  // the bus itself, e.g. I2C1
    GPIO_TypeDef *port;     // GPIO port both pins are on
    uint32_t scl_pin;       // clock, e.g. GPIO_PIN_8
    uint32_t sda_pin;       // data, e.g. GPIO_PIN_9
    uint32_t pin_af;        // alternate function that connects the pins to the bus
    uint32_t speed_hz;      // clock speed, e.g. 100000
} i2c_config_t;

/**
 * @brief Sets up an I2C bus, with this board as the one that starts every exchange.
 *
 * @param i2c    Filled in by this call; pass it to the other i2c_ functions
 * @param config Which bus, its pins and its speed
 * @return true if the bus was set up, false if the bus or port is not one this driver knows,
 *         or the HAL turned a setting down
 */
bool i2c_init(i2c_t *i2c, const i2c_config_t *config);

/**
 * @brief Says whether a device answers at an address.
 *
 * @param i2c     A bus set up with i2c_init()
 * @param address The device's 7-bit address
 * @return true if a device acknowledged the address
 */
bool i2c_device_present(i2c_t *i2c, uint8_t address);

/**
 * @brief Reads one or more bytes from a device, starting at one of its registers.
 *
 * @param i2c      A bus set up with i2c_init()
 * @param address  The device's 7-bit address
 * @param reg      The register to start at
 * @param data     Filled in with the bytes read
 * @param length   How many bytes to read
 * @return true if every byte was read, false if the device did not answer or it timed out
 */
bool i2c_read(i2c_t *i2c, uint8_t address, uint8_t reg, uint8_t *data, size_t length);

/**
 * @brief Writes one or more bytes to a device, starting at one of its registers.
 *
 * @param i2c      A bus set up with i2c_init()
 * @param address  The device's 7-bit address
 * @param reg      The register to start at
 * @param data     The bytes to write
 * @param length   How many bytes to write
 * @return true if every byte was written, false if the device did not answer or it timed out
 */
bool i2c_write(i2c_t *i2c, uint8_t address, uint8_t reg, const uint8_t *data, size_t length);

#endif
