/*
 * wifi.c
 *
 *  Created on: Dec 30, 2014
 *      Author: Minh
 */
#include "wifi.h"
#include "user_interface.h"
#include "osapi.h"
#include "espconn.h"
#include "os_type.h"
#include "mem.h"
#include "mqtt_msg.h"
//#include "debug.h"
#include "user_config.h"
#include "config.h"
//---------------------------------------------------------------------------

#define INFO
//#define INFO os_printf

//---------------------------------------------------------------------------

static ETSTimer WiFiLinker;
WifiCallback wifiCb = NULL;
static uint8_t wifiStatus = STATION_IDLE, lastWifiStatus = STATION_IDLE;

//---------------------------------------------------------------------------
static void ICACHE_FLASH_ATTR wifi_check_ip(void *arg)
{
	struct ip_info ipConfig;
	os_timer_disarm(&WiFiLinker);
	wifi_get_ip_info(STATION_IF, &ipConfig);
	wifiStatus = wifi_station_get_connect_status();
	if (wifiStatus == STATION_GOT_IP && ipConfig.ip.addr != 0)
	{
		os_timer_setfn(&WiFiLinker, (os_timer_func_t *)wifi_check_ip, NULL);
		os_timer_arm(&WiFiLinker, 2000, 0);
	}
	else
	{
		if(wifi_station_get_connect_status() == STATION_WRONG_PASSWORD)
		{
			INFO("STATION_WRONG_PASSWORD\r\n");
			wifi_station_connect();
		}
    else
     if (wifi_station_get_connect_status() == STATION_NO_AP_FOUND)
		{
			INFO("STATION_NO_AP_FOUND\r\n");
			wifi_station_connect();
		}
     else
      if (wifi_station_get_connect_status() == STATION_CONNECT_FAIL)
		{
			INFO("STATION_CONNECT_FAIL\r\n");
			wifi_station_connect();
		}
		else
		{
			INFO("STATION_IDLE\r\n");
		}

		os_timer_setfn(&WiFiLinker, (os_timer_func_t *)wifi_check_ip, NULL);
		os_timer_arm(&WiFiLinker, 500, 0);
	}
  if (wifiStatus != lastWifiStatus)
   {
		lastWifiStatus = wifiStatus;
    if (wifiCb) wifiCb(wifiStatus);
	}
}

//---------------------------------------------------------------------------
void ICACHE_FLASH_ATTR WIFI_Connect(uint8_t* ssid, uint8_t* pass, WifiCallback cb)
{
	struct station_config stationConf;

  INFO("WIFI_INIT %s:%s\r\n", ssid, pass);
  //wifi_set_opmode_current(STATION_MODE);
  wifi_set_opmode_current(STATIONAP_MODE);

	//wifi_station_set_auto_connect(FALSE);
	wifiCb = cb;

	os_memset(&stationConf, 0, sizeof(struct station_config));

	os_sprintf(stationConf.ssid, "%s", ssid);
	os_sprintf(stationConf.password, "%s", pass);

	wifi_station_set_config_current(&stationConf);

	os_timer_disarm(&WiFiLinker);
	os_timer_setfn(&WiFiLinker, (os_timer_func_t *)wifi_check_ip, NULL);
	os_timer_arm(&WiFiLinker, 1000, 0);

	//wifi_station_set_auto_connect(TRUE);
	wifi_station_connect();
}
//---------------------------------------------------------------------------

//#define AP_PWD
//#define MAC2PWD(a) (a)[0], (a)[1], (a)[2], (a)[5], (a)[4], (a)[3]
//#define MAC2PWD(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
void ICACHE_FLASH_ATTR setup_wifi_ap_mode(void)
{
    struct softap_config apconfig;
  //wifi_set_opmode((wifi_get_opmode() | SOFTAP_MODE) & STATIONAP_MODE);
  wifi_set_opmode_current(STATIONAP_MODE);
    if(wifi_softap_get_config(&apconfig))
    {
        wifi_station_dhcpc_stop();
        wifi_softap_dhcps_stop();
        char macaddr[6];
        wifi_get_macaddr(SOFTAP_IF, macaddr);
        os_memset(apconfig.ssid, 0, sizeof(apconfig.ssid));
        os_memset(apconfig.password, 0, sizeof(apconfig.password));
        apconfig.ssid_len = os_sprintf(apconfig.ssid, MQTT_DEV, MAC2STR(macaddr));
#ifdef MAC2PWD
        os_sprintf(apconfig.password, "%02x%02x%02x%02x%02x%02x", MAC2PWD(macaddr));
        apconfig.authmode = AUTH_WPA_WPA2_PSK;
#else
        apconfig.authmode = AUTH_OPEN;
#endif
        apconfig.ssid_hidden = 0;
        apconfig.channel = 7;
        apconfig.max_connection = 10;

        apconfig.beacon_interval = 1000;  // 1000 ms for lower power

        if(!wifi_softap_set_config(&apconfig))
         {
          INFO("GW not set AP config!\r\n");
         }
        struct ip_info ipinfo;
        if(wifi_get_ip_info(SOFTAP_IF, &ipinfo))
         {
            IP4_ADDR(&ipinfo.ip, 192, 168, 4, 1);
            IP4_ADDR(&ipinfo.gw, 192, 168, 4, 1);
            IP4_ADDR(&ipinfo.netmask, 255, 255, 255, 0);
            if(!wifi_set_ip_info(SOFTAP_IF, &ipinfo))
             {
              INFO("GW not set IP config!\r\n");
             }
            else
             {
              INFO("CONFIGURATION WEB SERVER IP: " IPSTR "\r\n", IP2STR(&ipinfo.ip));
             }
         }
    wifi_softap_dhcps_start();
    wifi_station_dhcpc_start();
    }
  if (wifi_get_phy_mode() != PHY_MODE_11N) wifi_set_phy_mode(PHY_MODE_11N);
  if (wifi_station_get_auto_connect() == 0) wifi_station_set_auto_connect(1);
  INFO("AP mode configured.\r\n");
    if(wifi_softap_get_config(&apconfig))
     {
      os_printf("AP config: SSID: %s, PASSWORD: %s, CHANNEL: %u\r\n", apconfig.ssid,  apconfig.password, apconfig.channel);
     }
}

//---------------------------------------------------------------------------
uint8_t ICACHE_FLASH_ATTR wifi_ap_channel(uint8_t channel)
 {
  struct softap_config apconfig;
  wifi_set_opmode_current(STATIONAP_MODE);
  wifi_softap_dhcps_stop();
  if (wifi_softap_get_config(&apconfig))
   {
    if (apconfig.channel != channel)
     {
      if (channel) apconfig.channel = channel;
      wifi_softap_set_config(&apconfig);
     }
   }
  INFO("CHANNEL: %u \r\n", apconfig.channel);
  return apconfig.channel;
 }
//---------------------------------------------------------------------------

