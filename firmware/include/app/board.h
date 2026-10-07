#ifndef BOARD_H
#define BOARD_H

#include <stdbool.h>

#include "i2c_driver.h"

// The set-up that hands the pins in board_config.h to the modules.

/**
 * @brief Sets up the HAL, the system clock, the user LED, the debug output and the link to the
 *        bridge, with the pins in board_config.h.
 *
 * Call once, first thing in main().
 *
 * @return true if every step worked
 */
bool board_init(void);

/**
 * @brief Gives the I2C bus the BME280 is on, for bme280_init().
 *
 * @return The bus, its pins and its speed
 */
const i2c_config_t *board_sensor_bus(void);

#endif
