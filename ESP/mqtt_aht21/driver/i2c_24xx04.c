/*
    Driver for the 24xx04 series serial EEPROM from ATMEL
    Official repository: https://github.com/CHERTS/esp8266-i2c_24xx32
    Base on https://github.com/husio-org/AT24C512C and https://code.google.com/p/arduino-at24c1024/
    This driver depends on the I2C driver https://github.com/zarya/esp8266_i2c_driver/

    Device start address 0x50 to 0x57

    Copyright (C) 2014 Mikhail Grigorev (CHERTS)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "driver/i2c.h"
#include "driver/i2c_24xx04.h"

#include "ets_sys.h"
#include "osapi.h"

/**
 * Read a single byte from the EEPROM
 * uint32_t  location   : The memory location to read
 */

uint8 ICACHE_FLASH_ATTR ee_b(uint32_t location)
 {
  uint8 data;
  uint8 control = 0xa0+(((location/256)&0x07)<<1);
  int tryes = 0;
  do
   {
    os_delay_us(I2C_SLEEP_TIME);
    system_soft_wdt_feed();
    i2c_start();
    i2c_writeByte(control);
    if (++tryes > 150)
     {
      i2c_stop();
      return 0;
     }
   }
  while (!i2c_check_ack());
  i2c_writeByte(location&255);
  if (!i2c_check_ack())
   {
    i2c_stop();
    return 0;
   }
  i2c_start();
  i2c_writeByte(control|1);
  if (!i2c_check_ack())
   {
    i2c_stop();
    return 0;
   }
  data = i2c_readByte();
  i2c_send_ack(0); // NOACK
  i2c_stop();
  return data;
 }
/**
 * Read multiple bytes from the EEPROM
 * uint32_t location   : The memory location to read
 * uint32_t len        : Number of bytes to read
 */

uint8 ICACHE_FLASH_ATTR ee_p(uint8 *data, uint32_t location, uint32_t len)
 {
  uint8 control = 0xa0+(((location/256)&0x07)<<1);
  int tryes = 0;
  do
   {
    os_delay_us(I2C_SLEEP_TIME);
    system_soft_wdt_feed();
    i2c_start();
    i2c_writeByte(control);
    if (++tryes > 150)
     {
      i2c_stop();
      return 0;
     }
   }
  while (!i2c_check_ack());
  i2c_writeByte(location&255);
  if (!i2c_check_ack())
   {
    i2c_stop();
    return 0;
   }
  i2c_start();
  i2c_writeByte(control|1);
  if (!i2c_check_ack())
   {
    i2c_stop();
    return 0;
   }

  uint32_t i;
  for (i = 0; i < len; i++)
   {
    system_soft_wdt_feed();
    data[i] = i2c_readByte();
    if(i != len-1) i2c_send_ack(1); //ACK
   }
  i2c_send_ack(0); // NOACK

  i2c_stop();
  return 1;
 }

/**
 * Write a byte to the I2C EEPROM
 * uint32_t location   : Memory location
 * uint8 data          : Data to write to the EEPROM
 */

uint8 ICACHE_FLASH_ATTR b_ee(uint8 data, uint32_t location)
 {
  uint8 control = 0xa0+(((location/256)&0x07)<<1);
  int tryes = 0;
  do
   {
    os_delay_us(I2C_SLEEP_TIME);
    system_soft_wdt_feed();
    i2c_start();
    i2c_writeByte(control);
    if (++tryes > 150)
     {
      i2c_stop();
      return 0;
     }
   }
  while (!i2c_check_ack());
  i2c_writeByte(location&255);
  if (!i2c_check_ack())
   {
    i2c_stop();
    return 0;
   }
  i2c_writeByte(data);
  if (!i2c_check_ack())
   {
    i2c_stop();
    return 0;
   }
  i2c_stop();
  return 1;
 }

/**
 * Write a page to the I2C EEPROM
 * uint32_t location   : Memory location
 * uint8 data          : Data to write to the EEPROM
 * uint32_t len        : The lenght of the data
 *
 */
uint8 ICACHE_FLASH_ATTR p_ee(uint8 *data, uint32_t location, uint32_t len)
 {
  uint8 control = 0xa0+(((location/256)&0x07)<<1);
  int tryes = 0;
  do
   {
    os_delay_us(I2C_SLEEP_TIME);
    system_soft_wdt_feed();
    i2c_start();
    i2c_writeByte(control);
    if (++tryes > 150)
     {
      i2c_stop();
      return 0;
     }
   }
  while (!i2c_check_ack());
  i2c_writeByte(location&255);
  if (!i2c_check_ack())
   {
    i2c_stop();
    return 0;
   }

  uint32_t i;
  for(i = 0; i < len; i++)
   {
    system_soft_wdt_feed();
    i2c_writeByte(data[i]);
    if (!i2c_check_ack())
     {
      i2c_stop();
      return 0;
     }
   }
  i2c_stop();
  return 1;
 }


// check for device existence
// return: 0 slave @ i2caddr does not exist, 1 slave @ i2caddr is present
uint8 ICACHE_FLASH_ATTR is_ee_exists(void)
 {
  os_delay_us(I2C_SLEEP_TIME);
  i2c_start();
  i2c_writeByte(0xa0);
  if (!i2c_check_ack())
   {
    i2c_stop();
    return 0;
   }
  else
   {
    i2c_stop();
    return 1;
   }
 }
