#include "bme280_raw.h"

void bme280_raw_from_bytes(const uint8_t *data, bme280_raw_t *raw)
{
    // Pressure and temperature are 20 bits each: 8 + 8 + the top 4 bits of a third byte
    raw->pressure = ((uint32_t)data[0] << 12) | ((uint32_t)data[1] << 4) | (data[2] >> 4);
    raw->temperature = ((uint32_t)data[3] << 12) | ((uint32_t)data[4] << 4) | (data[5] >> 4);
    raw->humidity = (uint16_t)((data[6] << 8) | data[7]);
}
