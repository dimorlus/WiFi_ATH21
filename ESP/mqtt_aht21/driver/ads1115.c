/*
 * ADS1115 driver for ESP8266 NON-OS SDK
 */

#include "driver/ads1115.h"
#include "driver/i2c.h"

#define ADS1115_REG_CONVERSION 0
#define ADS1115_REG_CONFIG 1
#define ADS1115_REG_LOW_THRESHOLD 2
#define ADS1115_REG_HIGH_THRESHOLD 3

#define ADS1115_CONFIG_OS 0x8000
#define ADS1115_CONFIG_MUX 0x7000
#define ADS1115_CONFIG_PGA 0x0E00
#define ADS1115_CONFIG_MODE 0x0100
#define ADS1115_CONFIG_DATA_RATE 0x00E0
#define ADS1115_CONFIG_COMP_MODE 0x0010
#define ADS1115_CONFIG_COMP_POLARITY 0x0008
#define ADS1115_CONFIG_COMP_LATCHING 0x0004
#define ADS1115_CONFIG_COMP_QUEUE 0x0003

static void ICACHE_FLASH_ATTR ads1115_update_config(ads1115_t *adc, uint8 shift,
                                                     uint16 mask, uint16 value);
static uint8 ICACHE_FLASH_ATTR ads1115_read_bits(uint16 value, uint16 mask, uint8 shift);
static ads1115_status_t ICACHE_FLASH_ATTR ads1115_write_register(ads1115_t *adc, uint8 reg,
                                                                  uint16 value);
static ads1115_status_t ICACHE_FLASH_ATTR ads1115_read_register(ads1115_t *adc, uint8 reg,
                                                                 uint16 *value);

ads1115_status_t ICACHE_FLASH_ATTR ads1115_init(ads1115_t *adc, uint8 address)
{
  if (adc == 0 || address < ADS1115_ADDRESS_GND || address > ADS1115_ADDRESS_SCL)
    return ADS1115_STATUS_I2C_NACK;

  i2c_init();
  adc->address = address;
  adc->config = 0;
  adc->pga = ADS1115_PGA_2_048V;

  ads1115_set_mux(adc, ADS1115_MUX_P0_GND);
  ads1115_set_pga(adc, ADS1115_PGA_2_048V);
  ads1115_set_mode(adc, ADS1115_MODE_SINGLE_SHOT);
  ads1115_set_data_rate(adc, ADS1115_RATE_128_SPS);
  ads1115_set_comparator_mode(adc, ADS1115_COMP_TRADITIONAL);
  ads1115_set_comparator_polarity(adc, ADS1115_COMP_ACTIVE_LOW);
  ads1115_set_comparator_latching(adc, ADS1115_COMP_NONLATCHING);
  ads1115_set_comparator_queue(adc, ADS1115_COMP_DISABLE);

  if (ads1115_upload_config(adc) != ADS1115_STATUS_OK)
    return ADS1115_STATUS_I2C_NACK;
  if (ads1115_set_low_threshold(adc, (sint16)0x8000) != ADS1115_STATUS_OK)
    return ADS1115_STATUS_I2C_NACK;
  if (ads1115_set_high_threshold(adc, 0x7FFF) != ADS1115_STATUS_OK)
    return ADS1115_STATUS_I2C_NACK;

  adc->pga = ADS1115_PGA_2_048V;
  return ADS1115_STATUS_OK;
}

uint8 ICACHE_FLASH_ATTR ads1115_is_connected(ads1115_t *adc)
{
  uint8 connected;

  if (adc == 0)
    return 0;

  i2c_start();
  i2c_writeByte(adc->address << 1);
  connected = i2c_check_ack();
  i2c_stop();
  return connected;
}

void ICACHE_FLASH_ATTR ads1115_set_mux(ads1115_t *adc, ads1115_mux_t mux)
{
  ads1115_update_config(adc, 12, ADS1115_CONFIG_MUX, (uint16)mux);
}

ads1115_status_t ICACHE_FLASH_ATTR ads1115_get_mux(ads1115_t *adc, ads1115_mux_t *mux)
{
  uint16 config;
  ads1115_status_t status;
  if (adc == 0 || mux == 0)
    return ADS1115_STATUS_I2C_NACK;
  status = ads1115_read_register(adc, ADS1115_REG_CONFIG, &config);
  if (status == ADS1115_STATUS_OK)
    *mux = (ads1115_mux_t)ads1115_read_bits(config, ADS1115_CONFIG_MUX, 12);
  return status;
}

void ICACHE_FLASH_ATTR ads1115_set_pga(ads1115_t *adc, ads1115_pga_t pga)
{
  ads1115_update_config(adc, 9, ADS1115_CONFIG_PGA, (uint16)pga);
}

ads1115_status_t ICACHE_FLASH_ATTR ads1115_get_pga(ads1115_t *adc, ads1115_pga_t *pga)
{
  uint16 config;
  ads1115_status_t status;
  if (adc == 0 || pga == 0)
    return ADS1115_STATUS_I2C_NACK;
  status = ads1115_read_register(adc, ADS1115_REG_CONFIG, &config);
  if (status == ADS1115_STATUS_OK) {
    *pga = (ads1115_pga_t)ads1115_read_bits(config, ADS1115_CONFIG_PGA, 9);
    adc->pga = *pga;
  }
  return status;
}

void ICACHE_FLASH_ATTR ads1115_set_mode(ads1115_t *adc, ads1115_mode_t mode)
{
  ads1115_update_config(adc, 8, ADS1115_CONFIG_MODE, (uint16)mode);
}

ads1115_status_t ICACHE_FLASH_ATTR ads1115_get_mode(ads1115_t *adc, ads1115_mode_t *mode)
{
  uint16 config;
  ads1115_status_t status;
  if (adc == 0 || mode == 0)
    return ADS1115_STATUS_I2C_NACK;
  status = ads1115_read_register(adc, ADS1115_REG_CONFIG, &config);
  if (status == ADS1115_STATUS_OK)
    *mode = (ads1115_mode_t)ads1115_read_bits(config, ADS1115_CONFIG_MODE, 8);
  return status;
}

void ICACHE_FLASH_ATTR ads1115_set_data_rate(ads1115_t *adc, ads1115_data_rate_t rate)
{
  ads1115_update_config(adc, 5, ADS1115_CONFIG_DATA_RATE, (uint16)rate);
}

ads1115_status_t ICACHE_FLASH_ATTR ads1115_get_data_rate(ads1115_t *adc,
                                                          ads1115_data_rate_t *rate)
{
  uint16 config;
  ads1115_status_t status;
  if (adc == 0 || rate == 0)
    return ADS1115_STATUS_I2C_NACK;
  status = ads1115_read_register(adc, ADS1115_REG_CONFIG, &config);
  if (status == ADS1115_STATUS_OK)
    *rate = (ads1115_data_rate_t)ads1115_read_bits(config, ADS1115_CONFIG_DATA_RATE, 5);
  return status;
}

void ICACHE_FLASH_ATTR ads1115_set_comparator_mode(ads1115_t *adc, ads1115_comp_mode_t mode)
{
  ads1115_update_config(adc, 4, ADS1115_CONFIG_COMP_MODE, (uint16)mode);
}

ads1115_status_t ICACHE_FLASH_ATTR ads1115_get_comparator_mode(ads1115_t *adc,
                                                                ads1115_comp_mode_t *mode)
{
  uint16 config;
  ads1115_status_t status;
  if (adc == 0 || mode == 0)
    return ADS1115_STATUS_I2C_NACK;
  status = ads1115_read_register(adc, ADS1115_REG_CONFIG, &config);
  if (status == ADS1115_STATUS_OK)
    *mode = (ads1115_comp_mode_t)ads1115_read_bits(config, ADS1115_CONFIG_COMP_MODE, 4);
  return status;
}

void ICACHE_FLASH_ATTR ads1115_set_comparator_polarity(ads1115_t *adc,
                                                        ads1115_comp_polarity_t polarity)
{
  ads1115_update_config(adc, 3, ADS1115_CONFIG_COMP_POLARITY, (uint16)polarity);
}

ads1115_status_t ICACHE_FLASH_ATTR ads1115_get_comparator_polarity(
  ads1115_t *adc, ads1115_comp_polarity_t *polarity)
{
  uint16 config;
  ads1115_status_t status;
  if (adc == 0 || polarity == 0)
    return ADS1115_STATUS_I2C_NACK;
  status = ads1115_read_register(adc, ADS1115_REG_CONFIG, &config);
  if (status == ADS1115_STATUS_OK)
    *polarity = (ads1115_comp_polarity_t)ads1115_read_bits(config, ADS1115_CONFIG_COMP_POLARITY, 3);
  return status;
}

void ICACHE_FLASH_ATTR ads1115_set_comparator_latching(ads1115_t *adc,
                                                        ads1115_comp_latching_t latching)
{
  ads1115_update_config(adc, 2, ADS1115_CONFIG_COMP_LATCHING, (uint16)latching);
}

ads1115_status_t ICACHE_FLASH_ATTR ads1115_get_comparator_latching(
  ads1115_t *adc, ads1115_comp_latching_t *latching)
{
  uint16 config;
  ads1115_status_t status;
  if (adc == 0 || latching == 0)
    return ADS1115_STATUS_I2C_NACK;
  status = ads1115_read_register(adc, ADS1115_REG_CONFIG, &config);
  if (status == ADS1115_STATUS_OK)
    *latching = (ads1115_comp_latching_t)ads1115_read_bits(config, ADS1115_CONFIG_COMP_LATCHING, 2);
  return status;
}

void ICACHE_FLASH_ATTR ads1115_set_comparator_queue(ads1115_t *adc, ads1115_comp_queue_t queue)
{
  ads1115_update_config(adc, 0, ADS1115_CONFIG_COMP_QUEUE, (uint16)queue);
}

ads1115_status_t ICACHE_FLASH_ATTR ads1115_get_comparator_queue(ads1115_t *adc,
                                                                 ads1115_comp_queue_t *queue)
{
  uint16 config;
  ads1115_status_t status;
  if (adc == 0 || queue == 0)
    return ADS1115_STATUS_I2C_NACK;
  status = ads1115_read_register(adc, ADS1115_REG_CONFIG, &config);
  if (status == ADS1115_STATUS_OK)
    *queue = (ads1115_comp_queue_t)ads1115_read_bits(config, ADS1115_CONFIG_COMP_QUEUE, 0);
  return status;
}

ads1115_status_t ICACHE_FLASH_ATTR ads1115_upload_config(ads1115_t *adc)
{
  ads1115_status_t status;
  if (adc == 0)
    return ADS1115_STATUS_I2C_NACK;
  status = ads1115_write_register(adc, ADS1115_REG_CONFIG, adc->config);
  if (status == ADS1115_STATUS_OK)
    adc->pga = (ads1115_pga_t)ads1115_read_bits(adc->config, ADS1115_CONFIG_PGA, 9);
  return status;
}

ads1115_status_t ICACHE_FLASH_ATTR ads1115_set_low_threshold(ads1115_t *adc, sint16 threshold)
{
  if (adc == 0)
    return ADS1115_STATUS_I2C_NACK;
  return ads1115_write_register(adc, ADS1115_REG_LOW_THRESHOLD, (uint16)threshold);
}

ads1115_status_t ICACHE_FLASH_ATTR ads1115_get_low_threshold(ads1115_t *adc, sint16 *threshold)
{
  uint16 value;
  ads1115_status_t status;
  if (adc == 0 || threshold == 0)
    return ADS1115_STATUS_I2C_NACK;
  status = ads1115_read_register(adc, ADS1115_REG_LOW_THRESHOLD, &value);
  if (status == ADS1115_STATUS_OK)
    *threshold = (sint16)value;
  return status;
}

ads1115_status_t ICACHE_FLASH_ATTR ads1115_set_high_threshold(ads1115_t *adc, sint16 threshold)
{
  if (adc == 0)
    return ADS1115_STATUS_I2C_NACK;
  return ads1115_write_register(adc, ADS1115_REG_HIGH_THRESHOLD, (uint16)threshold);
}

ads1115_status_t ICACHE_FLASH_ATTR ads1115_get_high_threshold(ads1115_t *adc, sint16 *threshold)
{
  uint16 value;
  ads1115_status_t status;
  if (adc == 0 || threshold == 0)
    return ADS1115_STATUS_I2C_NACK;
  status = ads1115_read_register(adc, ADS1115_REG_HIGH_THRESHOLD, &value);
  if (status == ADS1115_STATUS_OK)
    *threshold = (sint16)value;
  return status;
}

ads1115_status_t ICACHE_FLASH_ATTR ads1115_enable_conversion_ready_pin(ads1115_t *adc)
{
  ads1115_status_t status = ads1115_set_low_threshold(adc, 0);
  if (status != ADS1115_STATUS_OK)
    return status;
  return ads1115_set_high_threshold(adc, (sint16)0x8000);
}

ads1115_status_t ICACHE_FLASH_ATTR ads1115_start_single_shot(ads1115_t *adc)
{
  if (adc == 0)
    return ADS1115_STATUS_I2C_NACK;
  ads1115_update_config(adc, 15, ADS1115_CONFIG_OS, 1);
  return ads1115_upload_config(adc);
}

ads1115_status_t ICACHE_FLASH_ATTR ads1115_start_single_shot_channel(ads1115_t *adc,
                                                                      uint8 channel)
{
  if (channel > 3)
    return ADS1115_STATUS_CHANNEL_OUT_OF_RANGE;
  ads1115_set_mux(adc, (ads1115_mux_t)(ADS1115_MUX_P0_GND + channel));
  return ads1115_start_single_shot(adc);
}

ads1115_status_t ICACHE_FLASH_ATTR ads1115_start_continuous(ads1115_t *adc)
{
  if (adc == 0)
    return ADS1115_STATUS_I2C_NACK;
  ads1115_set_mode(adc, ADS1115_MODE_CONTINUOUS);
  return ads1115_upload_config(adc);
}

ads1115_status_t ICACHE_FLASH_ATTR ads1115_start_continuous_channel(ads1115_t *adc,
                                                                     uint8 channel)
{
  if (channel > 3)
    return ADS1115_STATUS_CHANNEL_OUT_OF_RANGE;
  ads1115_set_mux(adc, (ads1115_mux_t)(ADS1115_MUX_P0_GND + channel));
  return ads1115_start_continuous(adc);
}

ads1115_status_t ICACHE_FLASH_ATTR ads1115_is_conversion_ready(ads1115_t *adc, uint8 *ready)
{
  uint16 config;
  ads1115_status_t status;
  if (adc == 0 || ready == 0)
    return ADS1115_STATUS_I2C_NACK;
  status = ads1115_read_register(adc, ADS1115_REG_CONFIG, &config);
  if (status == ADS1115_STATUS_OK)
    *ready = (config & ADS1115_CONFIG_OS) == 0;
  return status;
}

ads1115_status_t ICACHE_FLASH_ATTR ads1115_read_conversion(ads1115_t *adc, sint16 *value)
{
  uint16 raw;
  ads1115_status_t status;
  if (adc == 0 || value == 0)
    return ADS1115_STATUS_I2C_NACK;
  status = ads1115_read_register(adc, ADS1115_REG_CONVERSION, &raw);
  if (status == ADS1115_STATUS_OK)
    *value = (sint16)raw;
  return status;
}

ads1115_status_t ICACHE_FLASH_ATTR ads1115_read_conversion_voltage(ads1115_t *adc, float *voltage)
{
  sint16 value;
  ads1115_status_t status;
  if (voltage == 0)
    return ADS1115_STATUS_I2C_NACK;
  status = ads1115_read_conversion(adc, &value);
  if (status == ADS1115_STATUS_OK)
    *voltage = ads1115_convert_to_voltage(adc, value);
  return status;
}

float ICACHE_FLASH_ATTR ads1115_convert_to_voltage(ads1115_t *adc, sint16 value)
{
  float lsb;
  if (adc == 0)
    return 0.0f;
  switch (adc->pga) {
    case ADS1115_PGA_0_256V: lsb = 0.256f / 32767.0f; break;
    case ADS1115_PGA_0_512V: lsb = 0.512f / 32767.0f; break;
    case ADS1115_PGA_1_024V: lsb = 1.024f / 32767.0f; break;
    case ADS1115_PGA_2_048V: lsb = 2.048f / 32767.0f; break;
    case ADS1115_PGA_4_096V: lsb = 4.096f / 32767.0f; break;
    case ADS1115_PGA_6_144V: lsb = 6.144f / 32767.0f; break;
    default: return 0.0f;
  }
  return value * lsb;
}

static void ICACHE_FLASH_ATTR ads1115_update_config(ads1115_t *adc, uint8 shift,
                                                     uint16 mask, uint16 value)
{
  if (adc == 0)
    return;
  adc->config &= (uint16)~mask;
  adc->config |= (uint16)(value << shift);
}

static uint8 ICACHE_FLASH_ATTR ads1115_read_bits(uint16 value, uint16 mask, uint8 shift)
{
  return (uint8)((value & mask) >> shift);
}

static ads1115_status_t ICACHE_FLASH_ATTR ads1115_write_register(ads1115_t *adc, uint8 reg,
                                                                  uint16 value)
{
  i2c_start();
  i2c_writeByte(adc->address << 1);
  if (!i2c_check_ack())
    goto nack;
  i2c_writeByte(reg);
  if (!i2c_check_ack())
    goto nack;
  i2c_writeByte((uint8)(value >> 8));
  if (!i2c_check_ack())
    goto nack;
  i2c_writeByte((uint8)value);
  if (!i2c_check_ack())
    goto nack;
  i2c_stop();
  return ADS1115_STATUS_OK;

nack:
  i2c_stop();
  return ADS1115_STATUS_I2C_NACK;
}

static ads1115_status_t ICACHE_FLASH_ATTR ads1115_read_register(ads1115_t *adc, uint8 reg,
                                                                 uint16 *value)
{
  uint16 result;

  i2c_start();
  i2c_writeByte(adc->address << 1);
  if (!i2c_check_ack())
    goto nack;
  i2c_writeByte(reg);
  if (!i2c_check_ack())
    goto nack;

  i2c_start();
  i2c_writeByte((adc->address << 1) | 1);
  if (!i2c_check_ack())
    goto nack;
  result = (uint16)i2c_readByte() << 8;
  i2c_send_ack(1);
  result |= i2c_readByte();
  i2c_send_ack(0);
  i2c_stop();

  *value = result;
  return ADS1115_STATUS_OK;

nack:
  i2c_stop();
  return ADS1115_STATUS_I2C_NACK;
}
