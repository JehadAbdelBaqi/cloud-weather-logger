#include "bme280.h"

#include "board.h"
#include "i2c.h"

// The two addresses the sensor can have, set by its address pin (BME280 datasheet, I2C interface)
#define ADDRESS_LOW  0x76
#define ADDRESS_HIGH 0x77

// Registers (BME280 datasheet section 5.4)
#define REG_CHIP_ID 0xD0

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
