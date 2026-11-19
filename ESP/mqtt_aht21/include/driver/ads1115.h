/*
 * ADS1115 driver for ESP8266 NON-OS SDK
 */

#ifndef __ADS1115_H__
#define __ADS1115_H__

#include "c_types.h"

#define ADS1115_ADDRESS_GND 0x48
#define ADS1115_ADDRESS_VDD 0x49
#define ADS1115_ADDRESS_SDA 0x4A
#define ADS1115_ADDRESS_SCL 0x4B

typedef enum {
  ADS1115_MUX_P0_N1 = 0,
  ADS1115_MUX_P0_N3,
  ADS1115_MUX_P1_N3,
  ADS1115_MUX_P2_N3,
  ADS1115_MUX_P0_GND,
  ADS1115_MUX_P1_GND,
  ADS1115_MUX_P2_GND,
  ADS1115_MUX_P3_GND
} ads1115_mux_t;

typedef enum {
  ADS1115_PGA_6_144V = 0,
  ADS1115_PGA_4_096V,
  ADS1115_PGA_2_048V,
  ADS1115_PGA_1_024V,
  ADS1115_PGA_0_512V,
  ADS1115_PGA_0_256V
} ads1115_pga_t;

typedef enum {
  ADS1115_MODE_CONTINUOUS = 0,
  ADS1115_MODE_SINGLE_SHOT
} ads1115_mode_t;

typedef enum {
  ADS1115_RATE_8_SPS = 0,
  ADS1115_RATE_16_SPS,
  ADS1115_RATE_32_SPS,
  ADS1115_RATE_64_SPS,
  ADS1115_RATE_128_SPS,
  ADS1115_RATE_250_SPS,
  ADS1115_RATE_475_SPS,
  ADS1115_RATE_860_SPS
} ads1115_data_rate_t;

typedef enum {
  ADS1115_COMP_TRADITIONAL = 0,
  ADS1115_COMP_WINDOW
} ads1115_comp_mode_t;

typedef enum {
  ADS1115_COMP_ACTIVE_LOW = 0,
  ADS1115_COMP_ACTIVE_HIGH
} ads1115_comp_polarity_t;

typedef enum {
  ADS1115_COMP_NONLATCHING = 0,
  ADS1115_COMP_LATCHING
} ads1115_comp_latching_t;

typedef enum {
  ADS1115_COMP_ASSERT_1 = 0,
  ADS1115_COMP_ASSERT_2,
  ADS1115_COMP_ASSERT_4,
  ADS1115_COMP_DISABLE
} ads1115_comp_queue_t;

typedef enum {
  ADS1115_STATUS_OK = 0,
  ADS1115_STATUS_I2C_NACK,
  ADS1115_STATUS_FAILED_TO_REQUEST_BYTES,
  ADS1115_STATUS_CHANNEL_OUT_OF_RANGE
} ads1115_status_t;

typedef struct {
  uint8 address;
  uint16 config;
  ads1115_pga_t pga;
} ads1115_t;

/* Initialize the I2C bus and configure the ADC with its default settings. */
ads1115_status_t ICACHE_FLASH_ATTR ads1115_init(ads1115_t *adc, uint8 address);
uint8 ICACHE_FLASH_ATTR ads1115_is_connected(ads1115_t *adc);

void ICACHE_FLASH_ATTR ads1115_set_mux(ads1115_t *adc, ads1115_mux_t mux);
ads1115_status_t ICACHE_FLASH_ATTR ads1115_get_mux(ads1115_t *adc, ads1115_mux_t *mux);
void ICACHE_FLASH_ATTR ads1115_set_pga(ads1115_t *adc, ads1115_pga_t pga);
ads1115_status_t ICACHE_FLASH_ATTR ads1115_get_pga(ads1115_t *adc, ads1115_pga_t *pga);
void ICACHE_FLASH_ATTR ads1115_set_mode(ads1115_t *adc, ads1115_mode_t mode);
ads1115_status_t ICACHE_FLASH_ATTR ads1115_get_mode(ads1115_t *adc, ads1115_mode_t *mode);
void ICACHE_FLASH_ATTR ads1115_set_data_rate(ads1115_t *adc, ads1115_data_rate_t rate);
ads1115_status_t ICACHE_FLASH_ATTR ads1115_get_data_rate(ads1115_t *adc, ads1115_data_rate_t *rate);
void ICACHE_FLASH_ATTR ads1115_set_comparator_mode(ads1115_t *adc, ads1115_comp_mode_t mode);
ads1115_status_t ICACHE_FLASH_ATTR ads1115_get_comparator_mode(ads1115_t *adc, ads1115_comp_mode_t *mode);
void ICACHE_FLASH_ATTR ads1115_set_comparator_polarity(ads1115_t *adc, ads1115_comp_polarity_t polarity);
ads1115_status_t ICACHE_FLASH_ATTR ads1115_get_comparator_polarity(ads1115_t *adc, ads1115_comp_polarity_t *polarity);
void ICACHE_FLASH_ATTR ads1115_set_comparator_latching(ads1115_t *adc, ads1115_comp_latching_t latching);
ads1115_status_t ICACHE_FLASH_ATTR ads1115_get_comparator_latching(ads1115_t *adc, ads1115_comp_latching_t *latching);
void ICACHE_FLASH_ATTR ads1115_set_comparator_queue(ads1115_t *adc, ads1115_comp_queue_t queue);
ads1115_status_t ICACHE_FLASH_ATTR ads1115_get_comparator_queue(ads1115_t *adc, ads1115_comp_queue_t *queue);

ads1115_status_t ICACHE_FLASH_ATTR ads1115_upload_config(ads1115_t *adc);
ads1115_status_t ICACHE_FLASH_ATTR ads1115_set_low_threshold(ads1115_t *adc, sint16 threshold);
ads1115_status_t ICACHE_FLASH_ATTR ads1115_get_low_threshold(ads1115_t *adc, sint16 *threshold);
ads1115_status_t ICACHE_FLASH_ATTR ads1115_set_high_threshold(ads1115_t *adc, sint16 threshold);
ads1115_status_t ICACHE_FLASH_ATTR ads1115_get_high_threshold(ads1115_t *adc, sint16 *threshold);
ads1115_status_t ICACHE_FLASH_ATTR ads1115_enable_conversion_ready_pin(ads1115_t *adc);
ads1115_status_t ICACHE_FLASH_ATTR ads1115_start_single_shot(ads1115_t *adc);
ads1115_status_t ICACHE_FLASH_ATTR ads1115_start_single_shot_channel(ads1115_t *adc, uint8 channel);
ads1115_status_t ICACHE_FLASH_ATTR ads1115_start_continuous(ads1115_t *adc);
ads1115_status_t ICACHE_FLASH_ATTR ads1115_start_continuous_channel(ads1115_t *adc, uint8 channel);
ads1115_status_t ICACHE_FLASH_ATTR ads1115_is_conversion_ready(ads1115_t *adc, uint8 *ready);
ads1115_status_t ICACHE_FLASH_ATTR ads1115_read_conversion(ads1115_t *adc, sint16 *value);
ads1115_status_t ICACHE_FLASH_ATTR ads1115_read_conversion_voltage(ads1115_t *adc, float *voltage);
float ICACHE_FLASH_ATTR ads1115_convert_to_voltage(ads1115_t *adc, sint16 value);

#endif
