#include "i2c_driver.h"

#include "gpio_driver.h"

// Longest wait for one exchange, in milliseconds
#define TIMEOUT_MS 100

// The HAL takes the address shifted up by one, leaving room for the read/write bit
#define HAL_ADDRESS(address) ((uint16_t)((address) << 1))

/**
 * @brief Turns on the clock of an I2C bus.
 *
 * @param instance The bus
 * @return true if the clock was turned on, false if the bus is not one this driver knows
 */
static bool bus_clock_enable(const I2C_TypeDef *instance)
{
    if (instance == I2C1) {
        __HAL_RCC_I2C1_CLK_ENABLE();
        return true;
    }
    return false;
}

/**
 * @brief Hands the clock and data pins to the bus.
 *
 * Open drain: a device only ever pulls a line low, and a pull-up brings it back high.
 * The internal pull-ups are switched on as well as any fitted on the device's board.
 *
 * @param config Which port, pins and alternate function
 * @return true if the pins were set up, false if the port is not one the GPIO driver knows
 */
static bool pins_init(const i2c_config_t *config)
{
    return gpio_init_peripheral_pins(config->port, config->scl_pin | config->sda_pin, GPIO_MODE_AF_OD, config->pin_af);
}

/**
 * @brief Sets the bus's speed and addressing and switches it on.
 *
 * @param i2c    Filled in by this call
 * @param config Which bus and its speed
 * @return true if the HAL accepted the settings
 */
static bool settings_init(i2c_t *i2c, const i2c_config_t *config)
{
    i2c->Instance = config->instance;
    i2c->Init.ClockSpeed = config->speed_hz;
    i2c->Init.DutyCycle = I2C_DUTYCYCLE_2;
    i2c->Init.OwnAddress1 = 0;
    i2c->Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    i2c->Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    i2c->Init.OwnAddress2 = 0;
    i2c->Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    i2c->Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    return HAL_I2C_Init(i2c) == HAL_OK;
}

bool i2c_init(i2c_t *i2c, const i2c_config_t *config)
{
    if (!bus_clock_enable(config->instance)) {
        return false;
    }
    if (!pins_init(config)) {
        return false;
    }
    return settings_init(i2c, config);
}

bool i2c_device_present(i2c_t *i2c, uint8_t address)
{
    return HAL_I2C_IsDeviceReady(i2c, HAL_ADDRESS(address), 1, TIMEOUT_MS) == HAL_OK;
}

bool i2c_read(i2c_t *i2c, uint8_t address, uint8_t reg, uint8_t *data, size_t length)
{
    return HAL_I2C_Mem_Read(i2c, HAL_ADDRESS(address), reg, I2C_MEMADD_SIZE_8BIT, data, length, TIMEOUT_MS) == HAL_OK;
}

bool i2c_write(i2c_t *i2c, uint8_t address, uint8_t reg, const uint8_t *data, size_t length)
{
    // The HAL's parameter is not const, though it only reads the bytes
    return HAL_I2C_Mem_Write(i2c, HAL_ADDRESS(address), reg, I2C_MEMADD_SIZE_8BIT, (uint8_t *)data, length, TIMEOUT_MS) == HAL_OK;
}
