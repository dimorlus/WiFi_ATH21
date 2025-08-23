/*
    Driver for the 24xx32 series serial EEPROM from ATMEL
    Official repository: https://github.com/CHERTS/esp8266-i2c_24xx32
    Base on https://github.com/husio-org/AT24C512C and https://code.google.com/p/arduino-at24c1024/
    This driver depends on the I2C driver https://github.com/zarya/esp8266_i2c_driver/

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

#ifndef __I2C_24XX04_H__
#define __I2C_24XX04_H__

#include "ets_sys.h"
#include "osapi.h"

/**
 * Read a single byte from the EEPROM
 * uint32_t  location   : The memory location to read
 */

extern uint8 ICACHE_FLASH_ATTR ee_b(uint32_t location);

/**
 * Write a byte to the I2C EEPROM
 * uint32_t location   : Memory location
 * uint8 data          : Data to write to the EEPROM
 */

extern uint8 ICACHE_FLASH_ATTR b_ee(uint8 data, uint32_t location);

/**
 * Read multiple bytes from the EEPROM
 * uint32_t location   : The memory location to read
 * uint32_t len        : Number of bytes to read
 */

extern uint8 ICACHE_FLASH_ATTR ee_p(uint8 *data, uint32_t location, uint32_t len);

/**
 * Write a page to the I2C EEPROM
 * uint32_t location   : Memory location
 * uint8 data          : Data to write to the EEPROM
 * uint32_t len        : The lenght of the data
 *
 */
extern uint8 ICACHE_FLASH_ATTR p_ee(uint8 *data, uint32_t location, uint32_t len);

// check for device existence
// return: 0 slave @ i2caddr does not exist, 1 slave @ i2caddr is present
extern uint8 ICACHE_FLASH_ATTR is_ee_exists(void);


#endif
