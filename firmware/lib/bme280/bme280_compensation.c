#include "bme280_compensation.h"

// The formula's highest humidity value, which is 100 %
#define HUMIDITY_MAX 419430400

int32_t bme280_compensate_temperature(uint32_t raw_temperature,
                                      const bme280_calibration_t *calibration, int32_t *t_fine)
{
    // BME280_compensate_T_int32 in the datasheet
    int32_t adc_t = (int32_t)raw_temperature;
    int32_t t1 = calibration->t1;
    int32_t t2 = calibration->t2;
    int32_t t3 = calibration->t3;

    int32_t var1 = (((adc_t >> 3) - (t1 << 1)) * t2) >> 11;
    int32_t var2 = (((((adc_t >> 4) - t1) * ((adc_t >> 4) - t1)) >> 12) * t3) >> 14;

    *t_fine = var1 + var2;
    return (*t_fine * 5 + 128) >> 8;
}

uint32_t bme280_compensate_pressure(uint32_t raw_pressure, const bme280_calibration_t *calibration,
                                    int32_t t_fine)
{
    // BME280_compensate_P_int64 in the datasheet
    const bme280_calibration_t *c = calibration;
    int64_t var1 = (int64_t)t_fine - 128000;
    int64_t var2 = var1 * var1 * (int64_t)c->p6;

    var2 = var2 + ((var1 * (int64_t)c->p5) << 17);
    var2 = var2 + ((int64_t)c->p4 << 35);
    var1 = ((var1 * var1 * (int64_t)c->p3) >> 8) + ((var1 * (int64_t)c->p2) << 12);
    var1 = ((((int64_t)1 << 47) + var1) * (int64_t)c->p1) >> 33;
    if (var1 == 0) {
        return 0;  // would divide by zero
    }

    int64_t p = 1048576 - (int64_t)raw_pressure;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = ((int64_t)c->p9 * (p >> 13) * (p >> 13)) >> 25;
    var2 = ((int64_t)c->p8 * p) >> 19;
    p = ((p + var1 + var2) >> 8) + ((int64_t)c->p7 << 4);

    return (uint32_t)(p >> 8);  // the formula gives pascals x 256
}

uint32_t bme280_compensate_humidity(uint16_t raw_humidity, const bme280_calibration_t *calibration,
                                    int32_t t_fine)
{
    // bme280_compensate_H_int32 in the datasheet; its one long line is split in two here
    int32_t adc_h = raw_humidity;
    int32_t h1 = calibration->h1;
    int32_t h2 = calibration->h2;
    int32_t h3 = calibration->h3;
    int32_t h4 = calibration->h4;
    int32_t h5 = calibration->h5;
    int32_t h6 = calibration->h6;

    int32_t v = t_fine - 76800;
    int32_t left = (((adc_h << 14) - (h4 << 20) - (h5 * v)) + 16384) >> 15;
    int32_t right = (((((((v * h6) >> 10) * (((v * h3) >> 11) + 32768)) >> 10) + 2097152) * h2) +
                     8192) >> 14;

    v = left * right;
    v = v - (((((v >> 15) * (v >> 15)) >> 7) * h1) >> 4);
    if (v < 0) {
        v = 0;
    }
    if (v > HUMIDITY_MAX) {
        v = HUMIDITY_MAX;
    }

    uint32_t percent_x1024 = (uint32_t)(v >> 12);  // the formula gives percent x 1024
    return (percent_x1024 * 100) >> 10;
}
