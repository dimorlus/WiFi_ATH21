/*
 * aht21b.c
 *
 *  Created on: Aug 18, 2025
 *      Author: Dmitry Orlov
 */

/*
 * AHT21B Sensor Driver for ESP8266 NON-OS SDK
 * Based on Thinary_AHT_Sensor Arduino library and provided i2c.c
 *
 * This driver implements basic initialization and reading of temperature,
 * humidity, and dew point for the AHT21B sensor using bit-banged I2C.
 *
 * Usage:
 * - Include this in your project along with i2c.c and i2c.h.
 * - Call aht21_init() once at startup.
 * - Use aht21_get_temperature(), aht21_get_humidity(), aht21_get_dew_point().
 *
 * Note: Error handling is minimal (returns -1 on failure). Add logging or
 * error callbacks as needed. Delays use os_delay_us().
 *
 * Constants like I2C_SDA_PIN and I2C_SCK_PIN must be defined in i2c.h.
 */

#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"
#include "mqtt_config.h"
#include "config.h"
#include "ets_sys.h"
#include "osapi.h"
#include "mqtt.h"
#include "wifi.h"
#include "config.h"
#include "debug.h"
#include "gpio.h"

#include "driver/i2c.h"  // Assuming i2c.h exists with declarations
#include "driver/aht21b.h"  // Assuming i2c.h exists with declarations


#define PRN
//#define PRN os_printf
// Sensor address (default for AHT21B)
#define AHT21_ADDRESS 0x38

// Commands from Arduino library
static uint8_t calibrate_cmd[3] =
 {0xE1, 0x08, 0x00};
static uint8_t measure_cmd[3] =
 {0xAC, 0x33, 0x00};
static uint8_t reset_cmd = 0xBA;

// Constants for dew point calculation
#define WATER_VAPOR 17.62f
#define BAROMETRIC_PRESSURE 243.5f

// Private function prototypes
static uint8_t aht21_read_status (void);
static void aht21_reset (void);
static int aht21_read_sensor (uint32_t *humidity_raw, uint32_t *temperature_raw);

// Initialize the sensor
int ICACHE_FLASH_ATTR aht21_init(void)
 {
  i2c_init();

  // Send calibration command
  i2c_start();
  i2c_writeByte(AHT21_ADDRESS << 1);// Write mode
  if (!i2c_check_ack())
   {
	i2c_stop();
	return -1;  // NACK error
   }
  i2c_writeByte(calibrate_cmd[0]);
  if (!i2c_check_ack())
   {
	i2c_stop();
	return -1;
   }
  i2c_writeByte(calibrate_cmd[1]);
  if (!i2c_check_ack())
   {
	i2c_stop();
	return -1;
   }
  i2c_writeByte(calibrate_cmd[2]);
  if (!i2c_check_ack())
   {
	i2c_stop();
	return -1;
   }
  i2c_stop();

  os_delay_us(500000);  // 500 ms delay

  // Check status
  uint8_t status = aht21_read_status();
  if ((status & 0x68) != 0x08)
   {
	return -1;  // Calibration failed
   }

  return 0;  // Success
 }


// Returns 0 - success, otherwise error.
uint8 ICACHE_FLASH_ATTR aht21b_read(float *temperature, float *humidity)
{
 uint32_t humidity_raw, temperature_raw;
 if (aht21_read_sensor(&humidity_raw, &temperature_raw) == 0)
  {
   *temperature = ((200.0f * (float)temperature_raw) / 1048576.0f) - 50.0f;
   *humidity = ((float)humidity_raw * 100.0f) / 1048576.0f;
   return 0;
  }
 *temperature = -1.0f;  // Error
 *humidity = -1.0f;  // Error
 return 1;
}

// Get temperature in degrees Celsius
float ICACHE_FLASH_ATTR aht21_get_temperature(void)
 {
  uint32_t humidity_raw, temperature_raw;
  if (aht21_read_sensor(&humidity_raw, &temperature_raw) != 0)
   {
	return -1.0f;  // Error
   }
  return ((200.0f * (float)temperature_raw) / 1048576.0f) - 50.0f;
 }

// Get relative humidity in %RH
float ICACHE_FLASH_ATTR aht21_get_humidity(void)
 {
  uint32_t humidity_raw, temperature_raw;
  if (aht21_read_sensor(&humidity_raw, &temperature_raw) != 0)
   {
	return -1.0f;  // Error
   }
  return ((float)humidity_raw * 100.0f) / 1048576.0f;
 }

static float ICACHE_FLASH_ATTR aht21_approx_log(float x)
 {
  if (x <= 0.0f)
   {
	return 0.0f;  // Invalid input
   }
  // Normalize x to use ln(1+z) series for 0 < x < 2
  float z = x - 1.0f;
  float result = 0.0f;
  float term = z;
  float z2 = z * z;
  int i;
  // Taylor series: ln(1+z) = z - z^2/2 + z^3/3 - z^4/4 + ...
  for (i = 1; i <= 10; i++)
   {  // Use 10 terms for reasonable accuracy
	result += term / i;
	term *= -z;
   }
  return result;
 }

// Get dew point in degrees Celsius
float ICACHE_FLASH_ATTR aht21_get_dew_point(float humidity, float temperature)
 {
  if (humidity < 0 || temperature < -50)
   {
	return -1.0f;  // Error in readings
   }

  float gamma = aht21_approx_log(humidity / 100.0f) + (WATER_VAPOR * temperature) / (BAROMETRIC_PRESSURE + temperature);
  float dew_point = (BAROMETRIC_PRESSURE * gamma) / (WATER_VAPOR - gamma);

  return dew_point;
 }

// Private: Read status byte
static uint8_t ICACHE_FLASH_ATTR aht21_read_status(void)
 {
  uint8_t status;

  i2c_start();
  i2c_writeByte((AHT21_ADDRESS << 1) | 1);  // Read mode
  if (!i2c_check_ack())
   {
	i2c_stop();
	return 0xFF;  // Error
   }
  status = i2c_readByte();
  i2c_send_ack(0);  // NACK for last byte
  i2c_stop();

  return status;
 }

// Private: Reset the sensor
static void ICACHE_FLASH_ATTR aht21_reset(void)
 {
  i2c_start();
  i2c_writeByte(AHT21_ADDRESS << 1);  // Write mode
  if (i2c_check_ack())
   {
	i2c_writeByte(reset_cmd);
	i2c_check_ack();  // Optional check
   }
  i2c_stop();
  os_delay_us(20000);  // 20 ms delay
 }

// Private: Trigger measurement and read raw data (returns 0 on success)
static int ICACHE_FLASH_ATTR aht21_read_sensor(uint32_t *humidity_raw, uint32_t *temperature_raw)
 {
  uint8_t data[6];
  int i;

  // Send measure command
  PRN("Send measure command\n");
  i2c_start();
  i2c_writeByte(AHT21_ADDRESS << 1);// Write mode
  if (!i2c_check_ack())
   {
	i2c_stop();
	return -1;
   }
  i2c_writeByte(measure_cmd[0]);
  if (!i2c_check_ack())
   {
	i2c_stop();
	return -1;
   }
  i2c_writeByte(measure_cmd[1]);
  if (!i2c_check_ack())
   {
	i2c_stop();
	return -1;
   }
  i2c_writeByte(measure_cmd[2]);
  if (!i2c_check_ack())
   {
	i2c_stop();
	return -1;
   }
  i2c_stop();

  os_delay_us(100000);  // 100 ms delay for measurement

  // Read 6 bytes
  PRN("Read 6 bytes\n");
  i2c_start();
  i2c_writeByte((AHT21_ADDRESS << 1) | 1);// Read mode
  if (!i2c_check_ack())
   {
	i2c_stop();
	return -1;
   }

  for (i = 0; i < 6; i++)
   {
	data[i] = i2c_readByte();
	PRN("%d:%d\n", i, data[i]);
	if (i < 5)
	 {
	  i2c_send_ack(1);  // ACK for next byte
	 }
	else
	 {
	  i2c_send_ack(0);  // NACK for last byte
	 }
   }
  i2c_stop();

  // Check if busy (status in data[0])
  if (data[0] & 0x80)
   {
	PRN("Busy\n");
	return -1;  // Busy
   }

  // Parse humidity (20 bits)
  *humidity_raw = ((uint32_t)data[1] << 12) | ((uint32_t)data[2] << 4) | ((uint32_t)data[3] >> 4);

  // Parse temperature (20 bits)
  *temperature_raw = (((uint32_t)data[3] & 0x0F) << 16) | ((uint32_t)data[4] << 8) | (uint32_t)data[5];
  PRN("%d:%d\n", *temperature_raw, *humidity_raw);
  return 0;
 }

#ifdef N2
#include "ets_sys.h"
#include "osapi.h"
#include "driver/aht21b.h"
#include "driver/i2c.h"

#define AHT21B_CMD_INIT       0xBE
#define AHT21B_CMD_MEASURE    0xAC
#define AHT21B_INIT_ARG1      0x08
#define AHT21B_INIT_ARG2      0x00
#define AHT21B_MEAS_ARG1      0x33
#define AHT21B_MEAS_ARG2      0x00

void aht21b_init(void) {
    // Простейшая инициализация
    i2c_start();
    i2c_writeByte((AHT21B_I2C_ADDRESS << 1)|0);
    i2c_writeByte(AHT21B_CMD_INIT);
    i2c_writeByte(AHT21B_INIT_ARG1);
    i2c_writeByte(AHT21B_INIT_ARG2);
    i2c_stop();
    os_delay_us(500000); // Задержка на инициализацию 500мс
}

// Возвращает 0 — успех, иначе ошибка.
uint8 aht21b_read(double *temperature, double *humidity) {
    uint8 i;
    uint8 raw[6];

    // Шаг 1: Запускаем измерение
    i2c_start();
    i2c_writeByte((AHT21B_I2C_ADDRESS << 1)|0);
    i2c_writeByte(AHT21B_CMD_MEASURE);
    i2c_writeByte(AHT21B_MEAS_ARG1);
    i2c_writeByte(AHT21B_MEAS_ARG2);
    i2c_stop();
    os_delay_us(80000); // Подождём ~80мс

    // Шаг 2: Читаем 6 байт
    i2c_start();
    i2c_writeByte((AHT21B_I2C_ADDRESS << 1)|1);
    for (i = 0; i < 6; i++) {
        raw[i] = i2c_readByte();
        if (i == 5) i2c_send_ack(0); // NACK на последний байт
        else        i2c_send_ack(1); // ACK на все кроме последнего
    }
    i2c_stop();

    // Проверяем статус
    if (raw[0] & 0x80) return 1; // busy

    // Собираем данные
    uint32 raw_h = (((uint32)raw[1] << 12) | ((uint32)raw[2] << 4) | ((uint32)raw[3] >> 4));
    uint32 raw_t = (((uint32)(raw[3] & 0x0F) << 16) | ((uint32)raw[4] << 8) | (uint32)raw[5]);

    *humidity = raw_h * 100.0 / 1048576.0;
    *temperature = raw_t * 200.0 / 1048576.0 - 50.0;
    return 0;
}

#endif //N2

