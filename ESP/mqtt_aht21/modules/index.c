#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"
#include "mem.h"
#include "espconn.h"
#include "httpd.h"
#include "config.h"
#include "mqtt_config.h"
#include "ver.h"
//#include "debug.h"


//#define INFO os_printf
#define INFO

#define ROM_ALLOC

#define KEY_SSID    "ssid"
#define KEY_PASS    "wifipass"
#define KEY_NAME    "devname"
#define KEY_PLACE   "place"
#define KEY_SERVER  "mqttsrv"
#define KEY_UDP     "mqttprt"
#define KEY_TLS     "tls_lvl"
#define KEY_HTOPIC  "htopic"
#define KEY_HUSR    "huser"
#define KEY_HPASS   "hpass"
#define KEY_CERT    "cert"
#define KEY_KEY     "key"
#define KEY_TZ      "tz"

#define CERT_SIZE   1600

//from user_main
extern void ICACHE_FLASH_ATTR OTA(void);
extern int ICACHE_FLASH_ATTR ada(void);
extern bool Connected;
extern int Connect;

extern bool SaveCFG;
extern bool LoadCFG;

#define _PTXT_ // enctype='text/plain'

#ifdef _PTXT_
#define MFORM "<form method='POST' enctype='text/plain'>"
#else
#define _SSID_PLUS_ //change '+' to ' ' in SSID
#define MFORM "<form method='POST'>"
#endif

#ifdef ROM_ALLOC
const char config_index1[] ICACHE_RODATA_ATTR STORE_ATTR =
#else
const char *config_index1 =
#endif
"<!doctype html>"\
"<html lang='en'>"\
"<head>"\
"<title>Device Configuration</title>"\
"<meta name='viewport' content='width=device-width, initial-scale=1' />"\
"<style>input[type='text'] {width: 100%;border-color: lightblue;}</style>"\
"</head>"\
"<body>"\
 MFORM\
"<TABLE BORDER='1'CELLPADDING='0' CELLSPACING='0'>"\
"<tr><td align='center'><h2><a href='app'>Device</a> config</h2></td></tr>"\
"<tr><td>APP %d Version : %s</td></tr>"\
"<tr><td>SDK Version : %s</td></tr>"\
"<tr><td>Dev tag : %s</td></tr>"\
"<tr><td>id: %08x, heap: %d</td></tr>"\
"<tr><TD>Type : <input type='text' name='"KEY_NAME"' value='%s'/></TD></tr>"\
"<tr><TD>Place : <input type='text' name='"KEY_PLACE"' value='%s'/></TD></tr>"\
"<tr><td align='center'><h3>Wi-Fi:%d</h3></td></tr>"\
"<tr><TD>SSID : <input type='text' name='"KEY_SSID"' value='%s'/></TD></tr>"\
"<tr><TD>Password : <input type='text' name='"KEY_PASS"' value='%s'/></TD></tr>"\
"<tr><TD>Time zone : <input type='text' name='"KEY_TZ"' value='%s'/></TD></tr>"\
"<tr><td align='center'><h3>Broker details: %d, %i</h3></td></tr>"\
"<tr><TD>Server : <input type='text' name='"KEY_SERVER"' value='%s'/></TD></tr>"\
"<tr><TD>Port : <input type='text' name='"KEY_UDP"' value='%d' size='15'/></TD></tr>"\
"<tr><TD>TLS : <input type='text' name='"KEY_TLS"' value='%d' size='15'/></TD></tr>"\
"<tr><TD align='center'><a href='certu'>Set CA certificate</a></TD></tr>"\
"<tr><TD align='center'><a href='keyu'>Set private key</a></TD></tr>"\
"<tr><TD>Topic base : <input type='text' name='"KEY_HTOPIC"' value='%s'/></TD></tr>"\
"<tr><TD>Broker User : <input type='text' name='"KEY_HUSR"' value='%s'/></TD></tr>"\
"<tr><TD>Broker Password : <input type='text' name='"KEY_HPASS"' value='%s'/></TD></tr>"\
"<tr><td align='center'>"\
"<input type='button' value='Reset' onclick=\"window.location.href='reset';\"/>&nbsp;&nbsp;"\
"<input type='submit' value='Save config'/>&nbsp;&nbsp;"\
"<input type='button' value='Factory default' onclick=\"window.location.href='clear';\"/>"\
"</td></tr>"\
"</TABLE>"\
"</form></body></html>";

//
//"<tr><td align='center'><input type='submit' value='Save config'/>&nbsp;&nbsp;&nbsp;<input type='button' value='Default'/></td></tr>"\
<input type="button" value="Clear" onclick="window.location.href='clear';"/>
#ifdef ROM_ALLOC
const char config_reset[] ICACHE_RODATA_ATTR STORE_ATTR =
#else
const char *config_reset =
#endif
"<!doctype html>"\
"<html lang='en'>"\
"<head>"\
"<title>Device Configration</title>"\
"<meta name='viewport' content='width=device-width, initial-scale=1'/>"\
"</head>"\
"<body><h2>Device config</h2>"\
"<form method='POST'>"\
"<div style='text-align: left;'>Settings saved. Do a "\
"<a style='background:#4E9CAF;padding:5px;color: white;' href='/reset'>reset</a>"\
" to apply them!</div>"\
"</form></body></html>";

#ifdef ROM_ALLOC
const char key_page[] ICACHE_RODATA_ATTR STORE_ATTR =
#else
const char *key_page =
#endif
"<!doctype html>"\
"<html lang='en'>"\
"<head>"\
"<title>Device Configuration</title>"\
"<meta name='viewport' content='width=device-width, initial-scale=1'/>"\
"<style>input[type='text'] {width: 100%;border-color: lightblue;}</style></head>"\
"<body>"\
"<form method='POST' enctype='text/plain'>"\
"<TABLE BORDER='1'CELLPADDING='0' CELLSPACING='0'>"\
"<tr><td align='center'><h3>Private key</h3>Paste Key :</td></tr>"\
"<tr><TD><textarea name='"KEY_KEY"' rows=5 cols=33>%s</textarea></TD></tr>"\
"<tr><td align='center'><input type='submit' value='Save key'/></td></tr>"\
"</TABLE>"\
"</form></body></html>";

#ifdef ROM_ALLOC
const char cert_page[] ICACHE_RODATA_ATTR STORE_ATTR =
#else
const char *cert_page =
#endif
"<!doctype html>"\
"<html lang='en'>"\
"<head>"\
"<title>Device Configuration</title>"\
"<meta name='viewport' content='width=device-width, initial-scale=1'/>"\
"<style>input[type='text'] {width: 100%;border-color: lightblue;}</style></head>"\
"<body>"\
"<form method='POST' enctype='text/plain'>"\
"<TABLE BORDER='1'CELLPADDING='0' CELLSPACING='0'>"\
"<tr><td align='center'><h3>CA certificate</h3>Paste Cert :</td></tr>"\
"<tr><TD><textarea name='"KEY_CERT"' rows=5 cols=33>%s</textarea></TD></tr>"\
"<tr><td align='center'><input type='submit' value='Save key'/></td></tr>"\
"</TABLE>"\
"</form></body></html>";

#ifdef ROM_ALLOC
const char idpg[] ICACHE_RODATA_ATTR STORE_ATTR =
#else
const char *idpg =
#endif
"{\"deviceId\":\"%s\",\"dtype\":\"%s\","DEVICE_CAPABILITIES",\"mData\":\"App=%d;ver=%s;SDK=%s;id= %08x;heap=%d\", \"ada\":%d}";

#ifdef ROM_ALLOC
void ICACHE_FLASH_ATTR rom_cpy(char *ram, const char *rom, int size)
 {
  int i;
  const uint32_t *rom32 = (const uint32_t *)rom;
  uint32_t *ram32 = (uint32_t *)ram;
  for(i = 0; i < (size/4)+1; i++) ram32[i] = rom32[i];
 }
#endif

#ifdef ROM_ALLOC1
inline static char ICACHE_FLASH_ATTR read_rom_char(const char* addr)
 {
  uint32_t bytes;
  bytes = *(uint32_t*) ((uint32_t) addr & ~3);
  return ((uint8_t*) &bytes)[(uint32_t) addr & 3];
 }

void ICACHE_FLASH_ATTR rom_cpy(char *ram, const char *rom, int size)
 {
  int i;
  for(i = 0; i < size; i++) ram[i] = read_rom_char(&rom[i]);
 }
#endif

static void ICACHE_FLASH_ATTR restart(void *arg)
 {
  system_restart();
 }

static bool ICACHE_FLASH_ATTR handleSettingsParameter(const char* key, const char* value)
 {
  //INFO("Param key=%s value=%s\r\n", key, value);

  if (*key == '\0') return false;

  if (strcmp(key, KEY_NAME) == 0)
   {
    if (strcmp(sysCfg.node_name, value))
     {
      os_strcpy(sysCfg.node_name, value);
      INFO("Changed key='%s' value='%s'\r\n", key, value);
      return true;
     }
   }
  else
  if (strcmp(key, KEY_PLACE) == 0)
   {
    if (strcmp(sysCfg.node_place, value))
     {
      os_strcpy(sysCfg.node_place, value);
      INFO("Changed key='%s' value='%s'\r\n", key, value);
      return true;
     }
   }
  else
  if (strcmp(key, KEY_SSID) == 0)
   {
    if (strcmp(sysCfg.sta_ssid, value))
     {
#ifdef _SSID_PLUS_
      const char *cps = value;
      char *cpd = sysCfg.sta_ssid;
      while (*cps)
       {
        if (*cps == '+')
         {
          *cpd++ = ' ';
          cps++;
         }
        else *cpd++ = *cps++;
       }
      *cpd = '\0';
#else
      os_strcpy(sysCfg.sta_ssid, value);
#endif
      INFO("Changed key='%s' value='%s'\r\n", key, value);
      return true;
     }
   }
  else
  if (strcmp(key, KEY_PASS) == 0)
   {
    if (strcmp(sysCfg.sta_pwd, value))
     {
      os_strcpy(sysCfg.sta_pwd, value);
      INFO("Changed key='%s' value='%s'\r\n", key, value);
      return true;
     }
   }
  else
  if (strcmp(key, KEY_TZ) == 0)
    {
     if (strcmp(sysCfg.TZ, value))
      {
       os_strcpy(sysCfg.TZ, value);
       INFO("Changed key='%s' value='%s'\r\n", key, value);
       return true;
      }
    }
   else
   if (strcmp(key, KEY_SERVER) == 0)
    {
     if (strcmp(sysCfg.mqtt_host, value))
      {
       os_strcpy(sysCfg.mqtt_host, value);
       INFO("Changed key='%s' value='%s'\r\n", key, value);
       return true;
      }
    }
   else
   if (strcmp(key, KEY_UDP) == 0)
    {
     int port = atoi(value);
     if (sysCfg.mqtt_port != port)
      {
       sysCfg.mqtt_port = port;
       INFO("Changed key='%s' value='%s'\r\n", key, value);
       return true;
      }
    }
   else
   if (strcmp(key, KEY_TLS) == 0)
    {
     int tls = atoi(value);
     if (sysCfg.security != tls)
      {
       sysCfg.security = tls;
       INFO("Changed key='%s' value='%s'\r\n", key, value);
       return true;
      }
    }
   else
   if (strcmp(key, KEY_HTOPIC) == 0)
    {
     if (strcmp(sysCfg.mqtt_topic_base, value))
      {
       os_strcpy(sysCfg.mqtt_topic_base, value);
       INFO("Changed key='%s' value='%s'\r\n", key, value);
       return true;
      }
    }
   else
   if (strcmp(key, KEY_HUSR) == 0)
    {
     if (strcmp(sysCfg.mqtt_user, value))
      {
       os_strcpy(sysCfg.mqtt_user, value);
       INFO("Changed key='%s' value='%s'\r\n", key, value);
       return true;
      }
    }
   else
   if (strcmp(key, KEY_HPASS) == 0)
    {
     if (strcmp(sysCfg.mqtt_pass, value))
      {
       os_strcpy(sysCfg.mqtt_pass, value);
       INFO("Changed key='%s' value='%s'\r\n", key, value);
       return true;
      }
    }
  INFO("No change key='%s' value='%s'\r\n", key, value);
  return false;
 }

#ifdef PRINT_CERT
ICACHE_FLASH_ATTR prn_cert(u8* cert)
 {
  int i;
  for(i=0; i < 1600; i++)
   {
    if (i%16==0) os_printf("\n\r%x: ", cert+i);
    os_printf("%02x ", cert[i]);
   }
  os_printf("\n");
 }
#else
#define prn_cert
#endif

static void ICACHE_FLASH_ATTR urldecode2(char *dst, const char *src)
 {
  char a, b;
  while (*src)
   {
    if ((*src == '%') && ((a = src[1]) && (b = src[2])) && (isxdigit(a) && isxdigit(b)))
     {
      if (a >= 'a') a -= 'a' - 'A';
      if (a >= 'A') a -= ('A' - 10);
      else a -= '0';
      if (b >= 'a') b -= 'a' - 'A';
      if (b >= 'A') b -= ('A' - 10);
      else b -= '0';
      *dst++ = 16 * a + b;
      src += 3;
     }
    else
#ifdef _PTXT_
    if ((*src == '\n')||(*src == '\r')) src++;
    else
#endif
     {
      *dst++ = *src++;
     }
   }
  *dst = '\0';
 }

bool ICACHE_FLASH_ATTR index_httpd_request(struct HttpdConnectionSlot *slot,
  uint8_t verb, char* path, uint8_t *data, uint16_t length)
 {
  INFO("verb=%d length=%d\r\n", verb, length);

  if (verb == HTTPD_VERB_POST)
   {
    data[length] = 0;
    INFO("\n%s[%d]:%s\n",path,length, data);
    if (strcasecmp(path, "/keyu") == 0)
     {
      char *cp = os_strstr(data, KEY_KEY"=");
      if (cp)
       {
        u32 *cert = os_zalloc(CERT_SIZE);
        os_memset(cert, 0xff, CERT_SIZE);
        int sz = b642data(cp + strlen(KEY_KEY"="), (u8*) cert);
        prn_cert((u8*) cert);
        spi_flash_erase_sector(CLIENT_CERT_FLASH_ADDRESS);
        spi_flash_write((CLIENT_CERT_FLASH_ADDRESS) * SPI_FLASH_SEC_SIZE, cert, sz);
        os_free(cert);
        httpd_send_text(slot, 200, "key_ok binary size %d bytes", sz);
       }
      else httpd_send_text(slot, 400, "key_error");
      httpd_send_text(slot, 200, "key_ok");
     }

    if (strcasecmp(path, "/certu") == 0)
     {
      char *cp = os_strstr(data, KEY_CERT"=");
      if (cp)
       {
        u32 *cert = os_zalloc(CERT_SIZE);
        os_memset(cert, 0xff, CERT_SIZE);
        int sz = b642data(cp + strlen(KEY_CERT"="), (u8*) cert);
        prn_cert((u8*) cert);
        spi_flash_erase_sector(CA_CERT_FLASH_ADDRESS);
        spi_flash_write((CA_CERT_FLASH_ADDRESS) * SPI_FLASH_SEC_SIZE, cert, sz);
        os_free(cert);
        httpd_send_text(slot, 200, "cert_ok binary size %d bytes", sz);
       }
      else httpd_send_text(slot, 400, "cert_error");
     }

    if ((strcasecmp(path, "/") == 0) ||
        (strcasecmp(path, "/save") == 0))
     {
      uint8_t pos = 0;
      char *key = data, *value;
      char decoded[48];
      bool changed = false;

#ifdef _PTXT_
      while (true)
       {
        if (pos == length || data[pos] == '\n')
         {
          data[pos] = 0;
          INFO("Key='%s' Val='%s'\n", key, value);
          urldecode2(decoded, value);
          changed |= handleSettingsParameter(key, decoded);
          if (pos < length) key = &data[++pos];
          else break;
          if (*key=='\0') break;
         }
        else
        if (data[pos] == '=')
         {
          data[pos] = 0;
          value = &data[pos + 1];
         }
        else pos++;
       }
#else
      while (true)
       {
        if (pos == length || data[pos] == '&')
         {
          data[pos] = 0;
          urldecode2(decoded, value);
          changed |= handleSettingsParameter(key, decoded);
          if (pos < length) key = &data[++pos];
          else break;
         }
        else
        if (data[pos] == '=')
         {
          data[pos] = 0;
          value = &data[pos + 1];
         }
        else pos++;
       }
#endif
      if (changed)
       {
        INFO("info changed. SAVE TO FLASH\r\n");
        //CFG_Save();
        SaveCFG = true;
        if (strcasecmp(path, "/") == 0)
         {
#ifdef ROM_ALLOC
          int sz = (sizeof(config_reset)-1) + (sizeof(config_reset)%4);
          char *cfg = (char*)os_zalloc(sz);
          rom_cpy(cfg, config_reset, sz);
          httpd_send_html(slot, 200, cfg);
          os_free(cfg);
#else
          httpd_send_html(slot, 200, config_reset);
#endif
         }
        if (strcasecmp(path, "/save") == 0)
         {
          httpd_send_text(slot, 200, "saved");
         }
       }
      else
       {
        httpd_send_text(slot, 200, "Settings did not change");
       }
     }
    else return false;

    return true;
   }

  if (strcasecmp(path, "/") == 0)
   {
    char str[48];
    char stag[18];
    char macaddr[6];
    //CFG_Load();
    LoadCFG = true;
#ifdef ROM_ALLOC
    int sz = (sizeof(config_index1)-1) + (sizeof(config_index1)%4);
    char *index = (char*)os_zalloc(sz);
#endif
    wifi_get_macaddr(STATION_IF, macaddr);
    os_sprintf(stag, MQTT_DEV, MAC2STR(macaddr));
    os_sprintf(str, _ver_"%d " __DATE__" "__TIME__, _build_);
    INFO("Send index.html ==>\r\n");
#ifdef ROM_ALLOC
    rom_cpy(index, config_index1, sz);
    httpd_send_html(slot, 200, index,
      _APP_,
      str,
      system_get_sdk_version(),
      stag,
      system_get_chip_id(),
      system_get_free_heap_size(),
      sysCfg.node_name,
      sysCfg.node_place,
      wifi_station_get_rssi(),
      sysCfg.sta_ssid,
      sysCfg.sta_pwd,
      sysCfg.TZ,
      Connected,
      Connect,
      sysCfg.mqtt_host,
      sysCfg.mqtt_port,
      sysCfg.security,
      sysCfg.mqtt_topic_base,
      sysCfg.mqtt_user,
      sysCfg.mqtt_pass);
    os_free(index);
#else
    httpd_send_html(slot, 200, config_index1,
      _APP_,
      str,
      system_get_sdk_version(),
      stag, system_get_chip_id(),
      system_get_free_heap_size(),
      sysCfg.node_name,
      sysCfg.node_place,
      wifi_station_get_rssi(),
      sysCfg.sta_ssid,
      sysCfg.sta_pwd,
      sysCfg.TZ,
      Connected,
      Connect,
      sysCfg.mqtt_host,
      sysCfg.mqtt_port,
      sysCfg.security,
      sysCfg.mqtt_topic_base,
      sysCfg.mqtt_user,
      sysCfg.mqtt_pass);
#endif
   }
  else
  if (strcasecmp(path, "/keyu") == 0)
   {
#ifdef ROM_ALLOC
    int sz = (sizeof(key_page)-1) + (sizeof(key_page)%4);
    char *cfg = (char*)os_zalloc(sz);
    rom_cpy(cfg, key_page, sz);
    httpd_send_html(slot, 200, cfg, "");
    os_free(cfg);
#else
    httpd_send_html(slot, 200, key_page, "");
#endif
   }
   if (strcasecmp(path, "/certu") == 0)
    {
 #ifdef ROM_ALLOC
     int sz = (sizeof(cert_page)-1) + (sizeof(cert_page)%4);
     char *cfg = (char*)os_zalloc(sz);
     rom_cpy(cfg, cert_page, sz);
     httpd_send_html(slot, 200, cfg, "");
     os_free(cfg);
 #else
     httpd_send_html(slot, 200, cert_page, "");
 #endif
    }

   if (strcasecmp(path, "/clear") == 0)
    {
     if (sysCfg.cfg_holder == CFG_HOLDER)
      {
       sysCfg.cfg_holder = 0;
       CFG_Save();
       httpd_send_text(slot, 200, "Settings cleared");
      }
     else
      {
       httpd_send_text(slot, 200, "No valid settings to clear");
      }
    }
   else
   if (strcasecmp(path, "/id") == 0)
    {
     char str[48];
     char stag[18];
     char macaddr[6];
#ifdef ROM_ALLOC
     int sz = (sizeof(idpg)-1) + (sizeof(idpg)%4);
     char *idp = (char*)os_zalloc(sz);
#endif
     wifi_get_macaddr(STATION_IF, macaddr);
     os_sprintf(stag, MQTT_DEV, MAC2STR(macaddr));
     os_sprintf(str, _ver_"%d " __DATE__" "__TIME__, _build_);
#ifdef ROM_ALLOC
     rom_cpy(idp, idpg, sz);
     httpd_send_text(slot, 200, idp,
       stag, sysCfg.node_name, _APP_, str, system_get_sdk_version(),
       system_get_chip_id(), system_get_free_heap_size(), ada());
     os_free(idp);
#else
     httpd_send_text(slot, 200, idpg,
       stag, sysCfg.node_name, _APP_, str, system_get_sdk_version(),
       system_get_chip_id(), system_get_free_heap_size(), ada());
#endif
    }
   else
   if (strcasecmp(path, "/heap") == 0)
    {
     httpd_send_text(slot, 200, "%d", system_get_free_heap_size());
    }
   else
   if (strcasecmp(path, "/reset") == 0)
    {
     httpd_send_text(slot, 200, "Resetting in 1sec");
     static ETSTimer restarttimer;
     os_timer_disarm(&restarttimer);
     os_timer_setfn(&restarttimer, (os_timer_func_t *) restart, NULL);
     os_timer_arm(&restarttimer, 1000, 0);
    }
   else
   if (strcasecmp(path, "/sdk") == 0)
    {
     httpd_send_text(slot, 200, system_get_sdk_version());
    }
   else
   if (strcasecmp(path, "/ver") == 0)
    {
     httpd_send_text(slot, 200, "Version "_ver_"%d " __DATE__" "__TIME__,
       _build_);
    }
   else
    {
     return false;
    }

  return true;
 }
