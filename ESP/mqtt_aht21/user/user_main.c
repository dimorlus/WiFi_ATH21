//RFID
/* main.c -- MQTT client example
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
#include "osapi.h"
#include "mqtt.h"
#include "wifi.h"
#include "config.h"
#include "debug.h"
#include "gpio.h"
#include "user_interface.h"
#include "mem.h"
#include "ota.h"
#include "sntp.h"
#include "driver/uart.h"
#include "driver/easygpio.h"
#include "driver/pwm.h"
#include "driver/aht21b.h"
#include "httpd.h"
#include "tsetup.h"


#include "ver.h"

#define PUBLISH_UPTIME_LIMIT    10 //sec
#define DISCONNECTED_TIMEOUT    30 //SEC - disconnected from Wi-Fi or TLS or MQTT
#define RESTART_TIME            60 //1min
#define MAC_MASK                0x1FF  //Post data if masked SNTP equal masked MAC (0x1ff == 8m 31s)
#define WCNT                    4


#define GW_BASE       //use gateway's MQTT base topic

//#define WIFI_EVENT          //debug wifi event


#define _I2C_ //24LC08
#define WEB //allow web interface at mqtt
#define AP  //allow SOFT AP at mqtt
#define _OTA_
#define SIC_TASK2   //Tags scan timer task

#define RLED        12	//status alive = once in sec, or fast blink in setup
#define GLED   	    13	//
#define KEY0        0   //button
#define PRN         os_printf
//#define PRN
//#define PRN1         os_printf
#define PRN1
//#define DEBUG
//#define PRN2 os_printf
#define PRN2
//---------------------------------------------------------------------------
//from index.c
bool ICACHE_FLASH_ATTR index_httpd_request(struct HttpdConnectionSlot *slot, uint8_t verb, char* path, uint8_t *data, uint16_t length);

//from user_main.c (this)
int ICACHE_FLASH_ATTR ada(void);
void ICACHE_FLASH_ATTR OTA(void);

//---------------------------------------------------------------------------

MQTT_Client mqttClient;

LOCAL os_timer_t _10ms_timer;
LOCAL os_timer_t _1s_timer;
#define _10MS_TIMER     10
#define _1S_TIMER     1000


LOCAL uint8_t led_state=0;
uint32_t _10ms = 0;
uint32_t rx_10ms_timeout = 0;

typedef enum
{
 cnNone = 0,
 cnWiFi = 1,
 cnIP = 2,
 cnNTP = 3,
 cnMQTT = 4
}tconnected;

bool SaveCFG = false;
bool LoadCFG = false;
bool WiFiCn = false;
bool Connected = false;
int Connect = cnNone;
bool SetupMode = false;
bool UpgradeRq = false;
bool inUpgrade = false;
bool PostFlag = false;
bool TSetup = false;
u32 disconnected_time=0;
u32 InfoFlag = 0;
static char otaUrl[64];
char str[400];
char topic[64];
char base_topic[48];
u32 sn; //serial number
u32 _1s = 0;
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//LOCAL void ICACHE_FLASH_ATTR dump(u8* data, int len)
// {
//  for(; len; len--) PRN("%02x ", *data++);
//  PRN("\n\r");
// }

//---------------------------------------------------------------------------
void ICACHE_FLASH_ATTR EnterSetup(void)
 {
  TSetup = true;
  MQTT_Disconnect(&mqttClient);
 }
//---------------------------------------------------------------------------
void ICACHE_FLASH_ATTR LeaveSetup(void)
 {
  TSetup = false;
  MQTT_Connect(&mqttClient);
 }
//---------------------------------------------------------------------------

LOCAL void ICACHE_FLASH_ATTR GetTimeDiff(void)
 {
  if (!Connected) return;
  os_sprintf(topic, "%s" TOPIC_TZR, base_topic);
  int sz = os_strlen(sysCfg.TZ);
  MQTT_Publish(&mqttClient, topic, sysCfg.TZ, sz, MQTT_QOS, 0);
  PRN("Pub: %s:%s\n\r",  topic, sysCfg.TZ);
 }
//---------------------------------------------------------------------------

int ICACHE_FLASH_ATTR Info(u16 wifi, char *sstr)
 {
  sstr[0] = '\0';
  char *cp = sstr;
  if (wifi & (1<<0))
   {
    char macaddr[6];
    wifi_get_macaddr(STATION_IF, macaddr);
    cp += os_sprintf(cp, "Dev: " MQTT_DEV ";", MAC2STR(macaddr));
   }
  if (wifi & (1<<1))
   {
    struct ip_info ipinfo;
    wifi_get_ip_info(0, &ipinfo);
    cp += os_sprintf(cp, "ip: %d.%d.%d.%d;rssi: %d;ch: %d;",
       IP2STR(&ipinfo.ip), wifi_station_get_rssi(), wifi_ap_channel(0));
   }
  if (wifi & (1<<2)) cp += os_sprintf(cp, "Type: %s;", sysCfg.node_name);
  if (wifi & (1<<3)) cp += os_sprintf(cp, "Place: %s;", sysCfg.node_place);
  if (wifi & (1<<4))
    cp += os_sprintf(cp, "Bld: App %d Version "_ver_"%d " __DATE__" "__TIME__";", _APP_, _build_);
  if (wifi & (1<<5)) cp += os_sprintf(cp, "SDK: %s;", system_get_sdk_version());
  if (wifi & (1<<6)) cp += os_sprintf(cp, "Sec: %d;", _1s);
  if (wifi & (1<<7)) cp += os_sprintf(cp, "SNTP: %u;", sntp_get_current_timestamp());
  if (wifi & (1<<8)) cp += os_sprintf(cp, "Heap: %d;", system_get_free_heap_size());
  if (wifi & (1<<9)) cp += os_sprintf(cp, "AD: %d;", ada());
  if (wifi & (1<<11)) cp += os_sprintf(cp, "TZD: %i;", sysCfg.tzDiff);
  if (wifi & (1<<12)) cp += os_sprintf(cp, "Connect: %i;", Connect);
  if (wifi & (1<<13)) cp += os_sprintf(cp, "CFG_HOLDER: %08X;", sysCfg.cfg_holder);
  return cp-sstr;
 }
//---------------------------------------------------------------------------
LOCAL void ICACHE_FLASH_ATTR custom_sleep (uint64_t sleep_us)
{
 while (sleep_us > 0)
  {
   uint32_t chunk = (sleep_us > 0xFFFFFFF) ? 0xFFFFFFF : (uint32_t) sleep_us;
   wifi_fpm_do_sleep(chunk);
   os_delay_us(10);  // Короткая задержка после wakeup для стабилизации
   sleep_us -= chunk;
  }
}
//---------------------------------------------------------------------------
LOCAL void ICACHE_FLASH_ATTR PostInfo(u16 wifi)
 {
  Info(wifi, str);
  os_sprintf(topic, "%s" TOPIC_INF, base_topic);
  PRN("Pub: %s:%s\n\r", topic, str);
  int sz = os_strlen(str);
  if (Connected) MQTT_Publish(&mqttClient, topic, str, sz, MQTT_QOS, 0);
 }
//---------------------------------------------------------------------------

LOCAL void ICACHE_FLASH_ATTR GetTopicBase(void)
 {
  int i;
  char macaddr[6];
  wifi_get_macaddr(STATION_IF, macaddr);

  for(i=0, sn = 0; i<6; i++) sn = sn*256+macaddr[i];
  sn /= 2;
  sn &= MAC_MASK;
  os_sprintf(base_topic, "%s/"MQTT_TOPIC_TYPE"/"MQTT_DEV, sysCfg.mqtt_topic_base, MAC2STR(macaddr));//deborah/RFID/3PI_94B97E1AB91C
 }
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void ICACHE_FLASH_ATTR alive(void)
 {
  if (!Connected) return;
  os_sprintf(topic, "%s" TOPIC_ALIVE, base_topic);
  Info(0xff7f, str);
  PRN("Pub: %s:%s\n\r", topic, str);
  int sz = os_strlen(str);
  MQTT_Publish(&mqttClient, topic, str, sz, MQTT_QOS, 0);
 }
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void ICACHE_FLASH_ATTR PostData(void)
 {
  float temp, hum;
  if (!Connected) return;

  uint32_t time = sntp_get_current_timestamp();

  if (aht21b_read(&temp, &hum) == 0)
   {
	float dew = aht21_get_dew_point(hum, temp);
    os_sprintf(str, "SNTP: %d, TEMP: %d.%d C, HUM: %d.%d %%, DEW: %d.%d C",
     time,
	 (int)temp, (int)(temp*10)%10,
	 (int)hum, (int)(hum*10)%10,
	 (int)dew, (int)(dew*10)%10);
   }
  else os_sprintf(str, "%d, AHT21B read error", time);

  os_sprintf(topic, "%s" TOPIC_HUMT, base_topic);
  PRN("Pub: %s:%s\n\r", topic, str);
  int sz = os_strlen(str);
  MQTT_Publish(&mqttClient, topic, str, sz, MQTT_QOS, 0);
 }
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
bool ICACHE_FLASH_ATTR isTopic(const char *rtopic, const char *topic)
{
 int ll = os_strlen(rtopic);
 int lr = os_strlen(topic);
 return (ll>=lr)&&(os_strcmp(&rtopic[ll-lr], topic)==0);
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

void ICACHE_FLASH_ATTR OTA(void)
 {
#ifdef _OTA_
  rx0_enable(false);
  clear_uart0();
  os_timer_disarm(&_10ms_timer);
  os_sprintf(str, "%s/user1.bin", otaUrl);
  PRN("OTA: %s\r\n", str);
  MQTT_Disconnect(&mqttClient);
  start_ota(str);
#endif
 }
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
LOCAL void ICACHE_FLASH_ATTR _1s_handler(void)
{
  static uint32_t RetryTime=0;
  _1s++;

  if (Connected||TSetup) disconnected_time = RetryTime = 0;
  else
   {
    if (!wifi_softap_get_station_num())
     {
      disconnected_time++;
      RetryTime++;
     }
    if (disconnected_time > 15)
    PRN("\033""7disconnected %5d\033""8", disconnected_time);
    if (RetryTime > 15)
     {
      RetryTime=0;
      PRN("Try connect MQTT\n");
      wifi_station_disconnect();
      wifi_station_connect();
     }
   }
  if (UpgradeRq)
   {
    UpgradeRq = false;
    inUpgrade = true;
    OTA();
   }

  if ((disconnected_time > DISCONNECTED_TIMEOUT))
   {
    PRN("Timeout\r\n");
    //system_restart();
   }
}

//---------------------------------------------------------------------------

LOCAL void ICACHE_FLASH_ATTR _1s_timer_cb(void *arg)
 {
  static uint32_t _1s = 0;
  static bool bsy = false;
  if (bsy) return;
  bsy = true;
  _1s++;
  system_soft_wdt_feed();
  if (inUpgrade) return;
  os_timer_disarm(&_1s_timer); //stop timer
  if (Connected)
   {
	if (_1s%600 == 0) PostData(); //10 minutes
   }

  os_timer_arm(&_1s_timer, _1S_TIMER, 1);//1s start timer
  bsy = false;
 }
//---------------------------------------------------------------------------

LOCAL void ICACHE_FLASH_ATTR _10ms_timer_cb(void *arg)
 {
  static bool bsy = false;
  static int Key = 0;
  _10ms++;
  rx_10ms_timeout++;
  if (bsy) return;
  bsy = true;
  system_soft_wdt_feed();
  if (inUpgrade) return;
  os_timer_disarm(&_10ms_timer); //stop timer

  if (easygpio_inputGet(KEY0))
   {
#ifdef GLED
    easygpio_outputSet(GLED, 0);
#endif
    Key = 0;
   }
  else
   {
#ifdef GLED
    easygpio_outputSet(GLED, 1);
#endif
    Key++;
   }

  if (Key > 500)
   {
    Key = 0;
#ifdef GLED
    easygpio_outputSet(GLED, 0);
#endif
    os_timer_disarm(&_10ms_timer); //stop timer
    Connected = false;
    CFG_Save();
    system_restart();
   }


  if (_10ms%100 == 0)
   {
    _1s_handler();
#ifdef RLED
    if (Connected) easygpio_outputSet(RLED, (led_state^=1));
#endif
   }

  if (SetupMode && (_10ms%20 == 0))
   {
#ifdef RLED
    easygpio_outputSet(RLED, (led_state^=1));
#endif
   }

  if ((!Connected)&&(_10ms%10 == 0))
   {
#ifdef RLED
    easygpio_outputSet(RLED, (led_state^=1));
#endif
   }

  os_timer_arm(&_10ms_timer, _10MS_TIMER, 1);//10ms start timer
  bsy = false;
 }

//---------------------------------------------------------------------------
LOCAL void ICACHE_FLASH_ATTR uart_cb(char c)
 {
  rx_10ms_timeout=0;
#ifdef GLED
  easygpio_outputSet(GLED, 1);
#endif
 }
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#ifdef PRINT_CERT
LOCAL void ICACHE_FLASH_ATTR print_ca(int sec)
 {
  int i;
  char *Buf = (char*)os_zalloc(4096);
  spi_flash_read(sec * SPI_FLASH_SEC_SIZE,
                     (uint32 *)Buf, SPI_FLASH_SEC_SIZE);
  PRN("%08x:", sec * SPI_FLASH_SEC_SIZE);
  for(i=0; i < SPI_FLASH_SEC_SIZE; i++)
   {
    if (i%16==0) PRN("\n\r%04x: ");
    PRN("%02x ", Buf[i]);
   }
  PRN("\n");
  os_free(Buf);
 }
#endif
//---------------------------------------------------------------------------

static ETSTimer ssdp_time_serv;
const char* const secures[] =
{
  "No TLS", // 0: disable SSL/TLS, there must be no certificate verify between MQTT server and ESP8266
  "TLS without authentication", // 1: enable SSL/TLS, but there is no a certificate verify
  "One way authentication", // 2: enable SSL/TLS, ESP8266 would verify the SSL server certificate at the same time
  "Two way authentication" // 3: enable SSL/TLS, ESP8266 would verify the SSL server certificate and SSL server would verify ESP8266 certificate
};
//---------------------------------------------------------------------------
void sntpfn()
{
 uint32_t ts = sntp_get_current_timestamp();
 PRN("CW:%d:%s\n", ts, sntp_get_real_time(ts));
 Connect = cnNTP;
 #ifdef PRINT_CERT
 print_ca(CA_CERT_FLASH_ADDRESS);
 print_ca(CLIENT_CERT_FLASH_ADDRESS);
 #endif
 if(ts && !TSetup)
  {
   os_timer_disarm(&ssdp_time_serv);
   PRN("Connecting to MQTT: %s:%d with %s\n\r", mqttClient.host, mqttClient.port, secures[mqttClient.security]);
   MQTT_Connect(&mqttClient);
  }
}
//---------------------------------------------------------------------------

void wifiConnectCb(uint8_t status)
{
  WiFiCn = (status == STATION_GOT_IP);
  Connect = cnWiFi;
  if(status == STATION_GOT_IP) // MQTT_Connect(&mqttClient);
   {
    Connect = cnIP;
    sntp_setservername(0, "pool.ntp.org");
    //sntp_setservername(0, "us.pool.ntp.org"); // set server 0 by domain name
    //sntp_setservername(1, "ntp.sjtu.edu.cn"); // set server 1 by domain name

    //sntp_set_timezone(sysCfg.tz); // Timezone control
    sntp_set_timezone(0); // Timezone control
    sntp_init();
    os_timer_disarm(&ssdp_time_serv);
    os_timer_setfn(&ssdp_time_serv, (os_timer_func_t *)sntpfn, NULL);
    os_timer_arm(&ssdp_time_serv, 1000, 1);//1s
   }
  else MQTT_Disconnect(&mqttClient);
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

void mqttConnectedCb(uint32_t *args)
{
 MQTT_Client* client = (MQTT_Client*)args;
 INFO("MQTT: Connected\r\n");

 //os_sprintf(topic, "%s/+" TOPIC_CFG, sysCfg.mqtt_topic_base);
 //MQTT_Subscribe(client, topic, 2);

 //subs to all devices command
 os_sprintf(topic, "%s/" MQTT_TOPIC_TYPE "/ALL" TOPIC_CFG, sysCfg.mqtt_topic_base);
 MQTT_Subscribe(client, topic, 2);
 PRN("Subscribe %s\n", topic);

 //subs to specific device command
 os_sprintf(topic, "%s" TOPIC_CFG, base_topic);
 MQTT_Subscribe(client, topic, 2);
 PRN("Subscribe %s\n", topic);

 os_sprintf(topic, "%s" TOPIC_SFG, base_topic);
 MQTT_Subscribe(client, topic, 2);
 PRN("Subscribe %s\n", topic);

 os_sprintf(topic, "%s" TOPIC_TZD, base_topic);
 MQTT_Subscribe(client, topic, 2);
 PRN("Subscribe %s\n", topic);

 os_sprintf(topic, "%s" TOPIC_FOTA, base_topic);
 MQTT_Subscribe(client, topic, 2);
 PRN("Subscribe %s\n", topic);

 Connected = true;
 Connect = cnMQTT;
 alive();
 PRN("MQTT(%s:%d): Connected (topic_base: %s)\r\n", sysCfg.mqtt_host, sysCfg.mqtt_port, base_topic);
 PostInfo(0x7fff);
 GetTimeDiff();
 os_timer_disarm(&_1s_timer);
 os_timer_setfn(&_1s_timer, (os_timer_func_t *)_1s_timer_cb, (void *)0);
 os_timer_arm(&_1s_timer, _1S_TIMER, 1);//1s start timer
 PostData();
}
//---------------------------------------------------------------------------

void mqttDisconnectedCb(uint32_t *args)
{
 MQTT_Client* client = (MQTT_Client*)args;
 INFO("MQTT: Disconnected\r\n");
 PRN("MQTT(%s:%d): Disconnected\r\n", sysCfg.mqtt_host, sysCfg.mqtt_port);
 Connected = false;
 Connect--;
 os_timer_disarm(&_1s_timer); //stop timer
}
//---------------------------------------------------------------------------

void mqttPublishedCb(uint32_t *args)
{
 MQTT_Client* client = (MQTT_Client*)args;
 INFO("MQTT: Published\r\n");
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

//MQTT data receive callback
void mqttDataCb(uint32_t *args, const char* rtopic, uint32_t topic_len, const char *data, uint32_t data_len)
{
 os_timer_disarm(&_10ms_timer); //stop timer

 char *topicBuf = (char*)os_zalloc(topic_len+1),
      *dataBuf = (char*)os_zalloc(data_len+1);

 MQTT_Client* client = (MQTT_Client*)args;

 os_memcpy(topicBuf, rtopic, topic_len);
 topicBuf[topic_len] = 0;

 os_memcpy(dataBuf, data, data_len);
 dataBuf[data_len] = 0;

 PRN("Receive topic: %s, len: %d, data: %s \r\n", topicBuf, data_len, dataBuf);

 os_sprintf(topic, "%s" TOPIC_FOTA, base_topic);
 if (os_strcmp(topicBuf, topic)==0)
  {
   os_strcpy(otaUrl, dataBuf);
   UpgradeRq = true;
   PRN("Upgrade request %s\n", otaUrl);
  }

 os_sprintf(topic, "%s" TOPIC_TZD, base_topic);
 {
  u32 param=0;
  if (isTopic(topicBuf, TOPIC_TZD) && (data_len == 8) && hex2dat(data, data_len, (u8*)&param))
   sysCfg.tzDiff = param;
 }

 os_sprintf(topic, "%s" TOPIC_CFG, base_topic);
 {
  u8 ucfg=0;
  u16 wifi=0;
  if (os_strcmp(topicBuf, topic)==0)
   {
     UpgradeRq = false;
     if ((data_len > 1)&&hex2dat(data, 2, &ucfg)&&(ucfg==0x55)) UpgradeRq = true;
   }
  if (isTopic(topicBuf, TOPIC_CFG) && (data_len == 4) && hex2dat(data, 4, (char*)&wifi))
   {
    //PRN("%s:%04X\r\n", topicBuf, wifi);
    PostInfo(wifi);
   }
 }

 os_sprintf(topic, "%s" TOPIC_SFG, base_topic);
 {
   if (os_strcmp(topicBuf, topic)==0)
    {
     parse("", 0L, 0);
     parse(dataBuf, str, sizeof(str)-1);
     os_sprintf(topic, "%s" TOPIC_ANS, base_topic);
     PRN("Pub: %s:%s\n\r", topic, str);
     int sz = os_strlen(str);
     if (Connected) MQTT_Publish(&mqttClient, topic, str, sz, MQTT_QOS, 0);
    }
 }



 os_free(topicBuf);
 os_free(dataBuf);
 os_timer_arm(&_10ms_timer, _10MS_TIMER, 1);//10ms start timer
}

//---------------------------------------------------------------------------
#ifdef V3x
#if ((SPI_FLASH_SIZE_MAP == 0) || (SPI_FLASH_SIZE_MAP == 1))
#error "The flash map is not supported"
#elif (SPI_FLASH_SIZE_MAP == 2)
#define SYSTEM_PARTITION_OTA_SIZE                           0x6A000
#define SYSTEM_PARTITION_OTA_2_ADDR                         0x81000
#define SYSTEM_PARTITION_RF_CAL_ADDR                        0xfb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR                      0xfc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR              0xfd000
#elif (SPI_FLASH_SIZE_MAP == 3)
#define SYSTEM_PARTITION_OTA_SIZE                           0x6A000
#define SYSTEM_PARTITION_OTA_2_ADDR                         0x81000
#define SYSTEM_PARTITION_RF_CAL_ADDR                        0x1fb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR                      0x1fc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR              0x1fd000
#elif (SPI_FLASH_SIZE_MAP == 4)
#define SYSTEM_PARTITION_OTA_SIZE                           0x6A000
#define SYSTEM_PARTITION_OTA_2_ADDR                         0x81000
#define SYSTEM_PARTITION_RF_CAL_ADDR                        0x3fb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR                      0x3fc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR              0x3fd000
#elif (SPI_FLASH_SIZE_MAP == 5)
#define SYSTEM_PARTITION_OTA_SIZE                           0x6A000
#define SYSTEM_PARTITION_OTA_2_ADDR                         0x101000
#define SYSTEM_PARTITION_RF_CAL_ADDR                        0x1fb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR                      0x1fc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR              0x1fd000
#elif (SPI_FLASH_SIZE_MAP == 6)
#define SYSTEM_PARTITION_OTA_SIZE                           0x6A000
#define SYSTEM_PARTITION_OTA_2_ADDR                         0x101000
#define SYSTEM_PARTITION_RF_CAL_ADDR                        0x3fb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR                      0x3fc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR              0x3fd000
#else
#error "The flash map is not supported"
#endif

static const partition_item_t at_partition_table[] = {
    { SYSTEM_PARTITION_BOOTLOADER,                      0x0,                                                0x1000},
    { SYSTEM_PARTITION_OTA_1,                           0x1000,                                             SYSTEM_PARTITION_OTA_SIZE},
    { SYSTEM_PARTITION_OTA_2,                           SYSTEM_PARTITION_OTA_2_ADDR,                        SYSTEM_PARTITION_OTA_SIZE},
    { SYSTEM_PARTITION_RF_CAL,                          SYSTEM_PARTITION_RF_CAL_ADDR,                       0x1000},
    { SYSTEM_PARTITION_PHY_DATA,                        SYSTEM_PARTITION_PHY_DATA_ADDR,                     0x1000},
    { SYSTEM_PARTITION_SYSTEM_PARAMETER,                SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR,             0x3000},
};

void ICACHE_FLASH_ATTR user_pre_init(void)
{
    if(!system_partition_table_regist(at_partition_table, sizeof(at_partition_table)/sizeof(at_partition_table[0]),SPI_FLASH_SIZE_MAP)) {
        //os_printf("system_partition_table_regist fail\r\n");
        while(1);
    }
}
#endif
//---------------------------------------------------------------------------
#ifdef V2x
//---------------------------------------------------------------------------
uint32 ICACHE_FLASH_ATTR user_rf_cal_sector_set(void)
{
 enum flash_size_map size_map = system_get_flash_size_map();
 uint32 rf_cal_sec = 0;

 switch (size_map)
  {
   case FLASH_SIZE_4M_MAP_256_256:
    rf_cal_sec = 128 - 5;
   break;

   case FLASH_SIZE_8M_MAP_512_512:
    rf_cal_sec = 256 - 5;
   break;

   case FLASH_SIZE_16M_MAP_512_512:
   case FLASH_SIZE_16M_MAP_1024_1024:
    rf_cal_sec = 512 - 5;
   break;

   case FLASH_SIZE_32M_MAP_512_512:
   case FLASH_SIZE_32M_MAP_1024_1024:
    rf_cal_sec = 1024 - 5;
   break;

   case FLASH_SIZE_64M_MAP_1024_1024:
    rf_cal_sec = 2048 - 5;
   break;

   case FLASH_SIZE_128M_MAP_1024_1024:
    rf_cal_sec = 4096 - 5;
   break;

   default:
    rf_cal_sec = 0;
   break;
  }
 return rf_cal_sec;
}
//---------------------------------------------------------------------------

void ICACHE_FLASH_ATTR user_rf_pre_init(void)
{
}
//---------------------------------------------------------------------------
#endif
int ICACHE_FLASH_ATTR ada(void)
 {
  int ad_tmp;
  int ad_avg=0;
  int i;

  for(i = 0; i < 64; i++)
   {
    ad_tmp = system_adc_read();
    ad_avg += (ad_tmp - ad_avg)/8;
    os_delay_us(100);
    system_soft_wdt_feed();
   }
  return ad_avg;
 }
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#ifdef WIFI_EVENT
/*
 * Call-back for changes in the WIFi connection's state.
 */
LOCAL void ICACHE_FLASH_ATTR wifi_event_cb(System_Event_t *event)
 {
  struct ip_info info;

  // To determine what actually happened, we need to look at the event.
  switch (event->event)
   {
   case EVENT_STAMODE_CONNECTED:
    {
     // We are connected as a station, but we don't have an IP address yet.
     char ssid[33];
     uint8_t len = event->event_info.connected.ssid_len;
     if (len > 32) len = 32;
     strncpy(ssid, event->event_info.connected.ssid, len + 1);
     os_printf("Received EVENT_STAMODE_CONNECTED. "
       "SSID = %s, BSSID = "MACSTR", channel = %d.\n", ssid, MAC2STR(event->event_info.connected.bssid),
       event->event_info.connected.channel);
     break;
    }
   case EVENT_STAMODE_DISCONNECTED:
    {
     // We have been disconnected as a station.
     char ssid[33];
     uint8_t len = event->event_info.connected.ssid_len;
     if (len > 32) len = 32;
     strncpy(ssid, event->event_info.connected.ssid, len + 1);
     os_printf("Received EVENT_STAMODE_DISCONNECTED. "
       "SSID = %s, BSSID = "MACSTR", channel = %d.\n", ssid, MAC2STR(event->event_info.disconnected.bssid),
       event->event_info.disconnected.reason);
     break;
    }
   case EVENT_STAMODE_GOT_IP:
    // We have an IP address, ready to run. Return the IP address, too.
    os_printf("Received EVENT_STAMODE_GOT_IP. IP = "IPSTR", mask = "IPSTR", gateway = "IPSTR"\n",
      IP2STR(&event->event_info.got_ip.ip.addr), IP2STR(&event->event_info.got_ip.mask.addr),
      IP2STR(&event->event_info.got_ip.gw));
   break;
   case EVENT_STAMODE_DHCP_TIMEOUT:
    // We couldn't get an IP address via DHCP, so we'll have to try re-connecting.
    os_printf("Received EVENT_STAMODE_DHCP_TIMEOUT.\n");
    wifi_station_disconnect();
    wifi_station_connect();
   break;
   case EVENT_SOFTAPMODE_STACONNECTED:
    os_printf("Received EVENT_SOFTAPMODE_STACONNECTED %d\n", wifi_softap_dhcps_status());
    wifi_softap_dhcps_stop();
    wifi_softap_dhcps_start();
   break;
   case EVENT_SOFTAPMODE_STADISCONNECTED:
    os_printf("Received EVENT_SOFTAPMODE_STADISCONNECTED\n");
   break;
   case EVENT_SOFTAPMODE_PROBEREQRECVED:
//  os_printf("Received EVENT_SOFTAPMODE_PROBEREQRECVED\n");
   break;
   case EVENT_OPMODE_CHANGED:
    os_printf("Received EVENT_OPMODE_CHANGED %d\n", wifi_get_opmode());
   break;
// case EVENT_SOFTAPMODE_DISTRIBUTE_STA_IP:
//  os_printf("Received EVENT_SOFTAPMODE_DISTRIBUTE_STA_IP\n");
// break;
   }
 }
#else
LOCAL void ICACHE_FLASH_ATTR wifi_event_cb(System_Event_t *event)
 {
  switch (event->event)
   {
    case EVENT_STAMODE_DHCP_TIMEOUT:
     // We couldn't get an IP address via DHCP, so we'll have to try re-connecting.
     wifi_station_disconnect();
     wifi_station_connect();
    break;
    case EVENT_SOFTAPMODE_STACONNECTED:
     //if (DHCP_STOPPED == wifi_softap_dhcps_status()) wifi_softap_dhcps_start();
     wifi_softap_dhcps_stop();
     wifi_softap_dhcps_start();
    break;
   }
 }
#endif
//---------------------------------------------------------------------------



void ICACHE_FLASH_ATTR user_init(void)
{
  int ad_val;
  Connected = false;

  // Configure pins as a GPIO
  uart_init(BIT_RATE_115200, BIT_RATE_115200);
  system_set_os_print(1);

  easygpio_pinMode(KEY0, EASYGPIO_PULLUP, EASYGPIO_INPUT);
#ifdef RLED
  easygpio_pinMode(RLED, EASYGPIO_NOPULL, EASYGPIO_OUTPUT);
  easygpio_outputSet(RLED, 0);
#endif
#ifdef GLED
  easygpio_pinMode(GLED, EASYGPIO_NOPULL, EASYGPIO_OUTPUT);
  easygpio_outputSet(GLED, 0);
#endif

  easygpio_pinMode(4, EASYGPIO_PULLUP, EASYGPIO_INPUT); //SCL
  easygpio_pinMode(5, EASYGPIO_PULLUP, EASYGPIO_INPUT); //SDA

  os_delay_us(60000); //60ms delay
  //system_update_cpu_freq(SYS_CPU_160MHZ);
  system_update_cpu_freq(SYS_CPU_80MHZ);

  wifi_fpm_set_sleep_type(LIGHT_SLEEP_T);
  wifi_fpm_open();  // Включить forced

  PRN("\033[2J\033[H");
  PRN("App %d Version "_ver_"%d " __DATE__" "__TIME__"\r\n", _APP_, _build_);

  i2c_init();
  //aht21b_init();

  aht21_init();
  CFG_Load();


  Connected = false;
  SetupMode = false;


  PRN("Device start\r\n");
  GetTopicBase();
  wifi_set_event_handler_cb(wifi_event_cb);

  {
    char macaddr[6];
    wifi_get_macaddr(STATION_IF, macaddr);
    os_sprintf(str, MQTT_DEV,  MAC2STR(macaddr));//deborah/RFID/3PI_94B97E1AB91C
  }
  MQTT_InitConnection(&mqttClient, sysCfg.mqtt_host, sysCfg.mqtt_port, sysCfg.security);
  MQTT_InitClient(&mqttClient, str, sysCfg.mqtt_user, sysCfg.mqtt_pass, sysCfg.mqtt_keepalive, 1);

  os_sprintf(topic, "%s" TOPIC_INIT, base_topic);
  Info((1<<3), str); //Place
  MQTT_InitLWT(&mqttClient, topic, str, MQTT_QOS, 0);
  MQTT_OnConnected(&mqttClient, mqttConnectedCb);
  MQTT_OnDisconnected(&mqttClient, mqttDisconnectedCb);
  MQTT_OnPublished(&mqttClient, mqttPublishedCb);
  MQTT_OnData(&mqttClient, mqttDataCb);

  WIFI_Connect(sysCfg.sta_ssid, sysCfg.sta_pwd, wifiConnectCb);

   {
    setup_wifi_ap_mode();
    #ifdef HTTP_SERVER_LOCAL_PORT
    httpd_register(index_httpd_request);
    httpd_init(HTTP_SERVER_LOCAL_PORT);
    #endif

    #ifdef TCP_SERVER_LOCAL_PORT
    user_tcpserver_init(TCP_SERVER_LOCAL_PORT);
    #endif
   }


  os_timer_disarm(&_10ms_timer);
  // os_timer_setfn(ETSTimer *ptimer, ETSTimerFunc *pfunction, void *parg)
  os_timer_setfn(&_10ms_timer, (os_timer_func_t *)_10ms_timer_cb, (void *)0);
  // void os_timer_arm(ETSTimer *ptimer,uint32_t milliseconds, bool repeat_flag)
  os_timer_arm(&_10ms_timer, _10MS_TIMER, 1);//10ms start timer


  set_uart_cb(uart_cb);

  PRN1("\r\nSystem started ...\r\n");
}
//---------------------------------------------------------------------------
