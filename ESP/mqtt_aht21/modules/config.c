/*
 /* config.c
 *
 * Copyright (c) 2014-2015, Tuan PM <tuanpm at live dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * * Neither the name of Redis nor the names of its contributors may be used
 * to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include "ets_sys.h"
#include "os_type.h"
#include "mem.h"
#include "osapi.h"
#include "user_interface.h"
#include "mqtt.h"
#include "config.h"
#include "user_config.h"
//#include "debug.h"

//#define INFO os_printf
#define INFO
//#define PRN         os_printf
#define PRN
//#define PRN1         os_printf
#define PRN1
//#define DEBUG
#define PRN2 os_printf
//#define PRN2

typedef struct {
    uint8 flag;
    uint8 pad[3];
} SAVE_FLAG;

SYSCFG sysCfg;
static SAVE_FLAG saveFlag;
static bool isEE = false;
//---------------------------------------------------------------------------
void ICACHE_FLASH_ATTR CfgEESave(void);
void ICACHE_FLASH_ATTR CfgEELoad(void);
//---------------------------------------------------------------------------
void ICACHE_FLASH_ATTR CFG_Save()
 {
  spi_flash_read((CFG_LOCATION + 3) * SPI_FLASH_SEC_SIZE, (uint32 *) &saveFlag, sizeof(SAVE_FLAG));

  if (saveFlag.flag == 0)
   {
    spi_flash_erase_sector(CFG_LOCATION + 1);
    spi_flash_write((CFG_LOCATION + 1) * SPI_FLASH_SEC_SIZE, (uint32 *) &sysCfg, sizeof(SYSCFG));
    saveFlag.flag = 1;
    spi_flash_erase_sector(CFG_LOCATION + 3);
    spi_flash_write((CFG_LOCATION + 3) * SPI_FLASH_SEC_SIZE, (uint32 *) &saveFlag, sizeof(SAVE_FLAG));
   }
  else
   {
    spi_flash_erase_sector(CFG_LOCATION + 0);
    spi_flash_write((CFG_LOCATION + 0) * SPI_FLASH_SEC_SIZE, (uint32 *) &sysCfg, sizeof(SYSCFG));
    saveFlag.flag = 0;
    spi_flash_erase_sector(CFG_LOCATION + 3);
    spi_flash_write((CFG_LOCATION + 3) * SPI_FLASH_SEC_SIZE, (uint32 *) &saveFlag, sizeof(SAVE_FLAG));
   }
 }

//---------------------------------------------------------------------------
void ICACHE_FLASH_ATTR  CfgPrn(void)
{
  os_printf("Config_size=%d\n\r", sizeof(sysCfg));
  os_printf("cfg_holder=%08X\n\r", sysCfg.cfg_holder);
  os_printf("sta_ssid=%s\n\r", sysCfg.sta_ssid);
  os_printf("sta_pwd=%s\n\r", sysCfg.sta_pwd);
  os_printf("sta_type=%d\n\r", sysCfg.sta_type);
  os_printf("mqtt_host=%s\n\r", sysCfg.mqtt_host);
  os_printf("mqtt_port=%d\n\r", sysCfg.mqtt_port);
  os_printf("mqtt_topic_base=%s\n\r", sysCfg.mqtt_topic_base);
  os_printf("mqtt_user=%s\n\r", sysCfg.mqtt_user);
  os_printf("mqtt_pass=%s\n\r", sysCfg.mqtt_pass);
  os_printf("node_name=%s\n\r", sysCfg.node_name);
  os_printf("node_place=%s\n\r", sysCfg.node_place);
  os_printf("TZ=%s\n\r", sysCfg.TZ);
  os_printf("mqtt_keepalive=%d\n\r", sysCfg.mqtt_keepalive);
  os_printf("security=%d\n\r", sysCfg.security);
  os_printf("utc=%d\n\r", sysCfg.utc);
  os_printf("tzDiff=%d\n\r", sysCfg.tzDiff);
}

LOCAL void ICACHE_FLASH_ATTR CfgDef(void)
{
 os_memset(&sysCfg, 0x00, sizeof sysCfg);

 sysCfg.cfg_holder = CFG_HOLDER;
 os_strncpy(sysCfg.sta_ssid, STA_SSID, sizeof(sysCfg.sta_ssid) - 1);
 os_strncpy(sysCfg.sta_pwd, STA_PASS, sizeof(sysCfg.sta_pwd) - 1);
 sysCfg.sta_type = STA_TYPE;
 os_strncpy(sysCfg.mqtt_host, MQTT_HOST, sizeof(sysCfg.mqtt_host) - 1);
 sysCfg.mqtt_port = MQTT_PORT;
 os_sprintf(sysCfg.mqtt_topic_base, "%s", MQTT_TOPIC_BASE);
 os_strncpy(sysCfg.mqtt_user, MQTT_USER, sizeof(sysCfg.mqtt_user) - 1);
 os_strncpy(sysCfg.mqtt_pass, MQTT_PASS, sizeof(sysCfg.mqtt_pass) - 1);
 os_sprintf(sysCfg.node_name, "%s", NODE_NAME);
 os_sprintf(sysCfg.node_place, "%s", PLACE);
 sysCfg.security = DEFAULT_SECURITY; /* default non ssl */
 sysCfg.mqtt_keepalive = MQTT_KEEPALIVE;
 sysCfg.utc = UTC_OFFSET;
 sysCfg.tzDiff = 3600*UTC_OFFSET;
 os_sprintf(sysCfg.TZ, "%s", TIMEZONE);
 PRN2("default configuration\r\n");
 CfgPrn();
}
//---------------------------------------------------------------------------

void ICACHE_FLASH_ATTR CFG_Load()
 {

  INFO("\r\nload ...\r\n");
  spi_flash_read((CFG_LOCATION + 3) * SPI_FLASH_SEC_SIZE, (uint32 *) &saveFlag, sizeof(SAVE_FLAG));
  if (saveFlag.flag == 0)
   {
    spi_flash_read((CFG_LOCATION + 0) * SPI_FLASH_SEC_SIZE, (uint32 *) &sysCfg, sizeof(SYSCFG));
   }
  else
   {
    spi_flash_read((CFG_LOCATION + 1) * SPI_FLASH_SEC_SIZE, (uint32 *) &sysCfg, sizeof(SYSCFG));
   }
  if (sysCfg.cfg_holder != CFG_HOLDER)
   {
    CfgDef();
    CFG_Save();
   }

 }
//---------------------------------------------------------------------------
