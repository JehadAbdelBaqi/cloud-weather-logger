#include "board.h"

#include "board_config.h"
#include "clock_driver.h"
#include "debug.h"
#include "led_driver.h"
#include "link.h"
#include "uart_driver.h"

// The link library sizes its buffer with its own limit: it has to be the one in board_config.h
_Static_assert(LINK_LINE_MAX_LEN == LINK_LINE_LIMIT,
               "the link library's line limit differs from LINK_LINE_LIMIT in board_config.h");

static const uart_config_t debug_config = {
    .instance = DEBUG_UART,
    .port = DEBUG_PORT,
    .tx_pin = DEBUG_TX_PIN,
    .rx_pin = 0,  // output only
    .pin_af = DEBUG_PIN_AF,
    .baud = DEBUG_BAUD,
};

static const uart_config_t link_config = {
    .instance = LINK_UART,
    .port = LINK_PORT,
    .tx_pin = LINK_TX_PIN,
    .rx_pin = LINK_RX_PIN,
    .pin_af = LINK_PIN_AF,
    .baud = LINK_BAUD,
};

static const i2c_config_t sensor_bus_config = {
    .instance = SENSOR_I2C,
    .port = SENSOR_PORT,
    .scl_pin = SENSOR_SCL_PIN,
    .sda_pin = SENSOR_SDA_PIN,
    .pin_af = SENSOR_PIN_AF,
    .speed_hz = SENSOR_I2C_SPEED_HZ,
};

bool board_init(void)
{
    HAL_Init();
    if (!clock_init()) {
        return false;
    }
    if (!led_init(LED_PORT, LED_PIN)) {
        return false;
    }
    if (!debug_init(&debug_config)) {
        return false;
    }
    return link_init(&link_config);
}

const i2c_config_t *board_sensor_bus(void)
{
    return &sensor_bus_config;
}
