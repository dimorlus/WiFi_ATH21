/*
 * aht21b.h
 *
 *  Created on: Aug 18, 2025
 *      Author: Dmitry Orlov
 */

#ifndef __AHT21B_H__
#define __AHT21B_H__

#include "c_types.h"

#define AHT21B_I2C_ADDRESS 0x38


// Initialize the sensor
int ICACHE_FLASH_ATTR aht21_init(void);

// Get temperature in degrees Celsius
float ICACHE_FLASH_ATTR aht21_get_temperature(void);

// Get relative humidity in %RH
float ICACHE_FLASH_ATTR aht21_get_humidity(void);

// Get dew point in degrees Celsius
float ICACHE_FLASH_ATTR aht21_get_dew_point(float humidity, float temperature);

//void aht21b_init(void);
//Returns 0 - success, otherwise error.
uint8 ICACHE_FLASH_ATTR aht21b_read(float *temperature, float *humidity);


#endif
