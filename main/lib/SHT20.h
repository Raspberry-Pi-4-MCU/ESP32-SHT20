#pragma once

// #ifndef _SHT20_H_
// #define _SHT20_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "driver/i2c.h"

/**
 * @brief	 SHT20 initialization
 *
 * @param  SDA  SDA pin in ESP32
 * @param  SCL  SCL pin in ESP32
 * 
 * @return
 *     - ESP_OK Success
 *     - ESP_ERR_INVALID_ARG GPIO error
 */
esp_err_t SHT20_init(int SDA, int SCL);

/**
 * @brief	 SHT20 read humidity by "no hold master" mode.  The resolution is 14 bits.
 * 
 * @return
 *     - Humidity
 */
float read_humidity(void);

/**
 * @brief	 SHT20 read temperature by "no hold master" mode.  The resolution is 14 bits.
 * 
 * @return
 *     - Temperature
 */
float read_temperature(void);

#ifdef __cplusplus
}
#endif

// #endif