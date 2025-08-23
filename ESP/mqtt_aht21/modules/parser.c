//---------------------------------------------------------------------------

#include "ets_sys.h"
#include "os_type.h"
#include "mem.h"
#include "osapi.h"
#include "user_interface.h"
#include "mqtt.h"
#include "config.h"
#include "user_config.h"


#include "mqtt_config.h"
#include "parser.h"
#include "config.h"
#include "datastr.h"
#include "strnum.h"
//#define PRN     os_printf
#define PRN


typedef enum
{
 tChr, tStr, tB64, tInt, tUInt, tFlt, tHex,
 tCmOn, tCmOff, tCmSave, tCmLoad, tCmRst,
 tCmHlp, tCmView, tCmInf
} types;

typedef enum {rError, rOk, rBOk} tres;

typedef struct
{
 char name[8];
 uint16_t type;
 uint16_t size;
 void *pval;
} crec;

//from user_main.c
int ICACHE_FLASH_ATTR Info(u16 wifi, char *sstr);
//---------------------------------------------------------------------------

static const crec Recs[] =
{
 {"SIGN", tHex, sizeof(sysCfg.cfg_holder), &sysCfg.cfg_holder},
 {"NAME", tStr, sizeof(sysCfg.node_name), &sysCfg.node_name},
 {"SSID", tStr, sizeof(sysCfg.sta_ssid), &sysCfg.sta_ssid},
 {"PWD", tStr, sizeof(sysCfg.sta_pwd), &sysCfg.sta_pwd},
 {"PLACE", tStr, sizeof(sysCfg.node_place), &sysCfg.node_place},
 {"TZ", tStr, sizeof(sysCfg.TZ), &sysCfg.TZ},
 {"MQTT", tStr, sizeof(sysCfg.mqtt_host), &sysCfg.mqtt_host},
 {"MPORT", tUInt, sizeof(sysCfg.mqtt_port), &sysCfg.mqtt_port},
 {"TLS", tUInt, sizeof(sysCfg.security), &sysCfg.security},
 {"KEY", tB64, 1600, 0L},
 {"CERT", tB64, 1600, 0L},
 {"BASE", tStr, sizeof(sysCfg.mqtt_topic_base), &sysCfg.mqtt_topic_base},
 {"USER", tStr, sizeof(sysCfg.mqtt_user), &sysCfg.mqtt_user},
 {"UPWD", tStr, sizeof(sysCfg.mqtt_pass), &sysCfg.mqtt_pass},
 {"HELP", tCmHlp, 0, 0L},
 {"VIEW", tCmView, 0, 0L},
 {"SAVE", tCmSave, 0, 0L},
 {"LOAD", tCmLoad, 0, 0L},
 {"RESET", tCmRst, 0, 0L},
 {"INFO", tCmInf, 0, 0L},
};
const RECS = sizeof(Recs)/sizeof(crec);
//---------------------------------------------------------------------------

extern bool SaveCFG;
extern bool LoadCFG;

//---------------------------------------------------------------------------
LOCAL int ICACHE_FLASH_ATTR fSaveCfg(void)
{
  CFG_Save();
  SaveCFG = true;
  return 0;
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
LOCAL int ICACHE_FLASH_ATTR fLoadCfg(void)
{
  CFG_Load();
  LoadCFG = true;
  return 0;
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

LOCAL int ICACHE_FLASH_ATTR vlen(int i)
{
 switch(Recs[i].type)
  {
   case tCmOn:
   case tCmOff:
   case tCmSave:
   case tCmLoad:
   case tCmRst:
   case tCmHlp:
   case tCmView:
    return os_strlen(Recs[i].name);
   case tChr:
    return os_strlen(Recs[i].name)+3;
   case tStr:
    return os_strlen(Recs[i].name)+os_strlen((char*)Recs[i].pval)+1;
   case tB64:
    return os_strlen(Recs[i].name)+1;
   case tInt:
   case tUInt:
   case tFlt:
   case tHex:
    return os_strlen(Recs[i].name)+10;
  }
 return 0;
}

//---------------------------------------------------------------------------
LOCAL int ICACHE_FLASH_ATTR hlen(int i)
{
   switch(Recs[i].type)
    {
     case tCmOn:
     case tCmOff:
     case tCmSave:
     case tCmLoad:
     case tCmRst:
     case tCmHlp:
     case tCmView:
      return os_strlen(Recs[i].name);
     case tChr:
     case tStr:
     case tB64:
     case tInt:
     case tUInt:
     case tFlt:
     case tHex:
      return os_strlen(Recs[i].name)+16;
    }
  return 0;  
}

//---------------------------------------------------------------------------

LOCAL int ICACHE_FLASH_ATTR help(char *bres, int len)
{
 char *pstr = bres;
 int i;
 for(i = 0; i < RECS; i++)
  {
   if (pstr-bres+hlen(i)+4 > len) break;
   pstr += os_sprintf(pstr, "%s", Recs[i].name);
   switch(Recs[i].type)
    {
     case tCmOn:
     case tCmOff:
     case tCmSave:
     case tCmLoad:
     case tCmRst:
     case tCmHlp:
     case tCmView:
     break;
     case tChr:
      pstr += os_sprintf(pstr, "= Char value");
     break;
     case tStr:
      pstr += os_sprintf(pstr, "= String value");
     break;
     case tB64:
      pstr += os_sprintf(pstr, "= Base64 value");
     break;
     case tInt:
     case tUInt:
      pstr += os_sprintf(pstr, "= Integer value");
     break;
     case tFlt:
      pstr += os_sprintf(pstr, "= Floating point value");
     break;
     case tHex:
      pstr += os_sprintf(pstr, "= HEX value");
     break;
    }
   pstr += os_sprintf(pstr, "\n\r");
  }
 return pstr-bres;
}
//---------------------------------------------------------------------------
static int ICACHE_FLASH_ATTR view(char *bres, int len);
//---------------------------------------------------------------------------
static int ICACHE_FLASH_ATTR inf(char *bres, int len)
 {
  int i;
  uint32_t mask = 1;
  char *pstr = bres;
  for(i=0; i<16; i++)
   {
    if (pstr-bres+16 > len) break;
    pstr += Info(mask, pstr);
    pstr += os_sprintf(pstr, "\n\r");
    mask <<= 1;
   }
  return pstr-bres;
 }
//---------------------------------------------------------------------------

static int ICACHE_FLASH_ATTR command(int i, char *bres, int len)
{
 char *pstr = bres;
 switch(Recs[i].type)
  {
   case tCmSave:
    pstr += fSaveCfg();
   break;
   case tCmLoad:
    pstr += fLoadCfg();
   break;
   case tCmHlp:
    pstr += help(bres, len);
   break;
   case tCmView:
    pstr += view(bres, len);
   break;
   case tCmInf:
    pstr += inf(bres, len);
   break;
   case tCmRst:
    system_restart();
   break;
   default: pstr += os_sprintf(pstr, "ERROR\n\r");
  }
 return pstr-bres;
}
//---------------------------------------------------------------------------
#define T_CLIENT_CERT_FLASH_ADDRESS 0x76
static tres ICACHE_FLASH_ATTR save_dat(int i, uint8_t* buf, uint32_t size)
{
 //dump(buf, size);

 if (size > 2)
  {
   int i;
   uint8_t A, B;
   A = B = 0x55;
   for(i = 0; i < size-2; i++) A += B += buf[i];
   PRN("Chk:%02x%02x -> %02x%02x\n", A, B, buf[size-2], buf[size-1]);
   if ((A != buf[size-2]) || (B != buf[size-1])) return rError;
   buf[size-2] = buf[size-1] = 0xff;
  }
 else return rError;

#ifdef _Windows
 if (Recs[i].name[0] == 'K') Save(key_name, buf, Recs[i].size); //Key
 else Save(crt_name, buf, Recs[i].size);
#else
 if (Recs[i].name[0] == 'K')
  {
   spi_flash_erase_sector(CLIENT_CERT_FLASH_ADDRESS);
   spi_flash_write((CLIENT_CERT_FLASH_ADDRESS) * SPI_FLASH_SEC_SIZE, (uint32_t *)buf, size-2);
  }
 else
  {
   spi_flash_erase_sector(CA_CERT_FLASH_ADDRESS);
   spi_flash_write((CA_CERT_FLASH_ADDRESS) * SPI_FLASH_SEC_SIZE, (uint32_t *)buf, size-2);
  }
#endif
 return rOk;
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

static  uint32_t ICACHE_FLASH_ATTR ICACHE_FLASH_ATTR htoi(const char* str)
{
 uint32_t res = 0;
 char c;
 int n = 0;
 while (c = *str++, c && (n < 16))
  {
   if ((c >= '0') && (c <= '9')) {res = res * 16 + (c - '0'); n++;}
   else
   if ((c >= 'A') && (c <= 'F')) {res = res * 16 + (c - 'A') + 0xA; n++;}
   else
   if ((c >= 'a') && (c <= 'f')) {res = res * 16 + (c - 'a') + 0xa; n++;}
   else break;
  }
 return res;
}

//---------------------------------------------------------------------------
static int ICACHE_FLASH_ATTR display(int i, char *bres, int len)
{
 char *pres = bres;
 pres += os_sprintf(pres, "%s=", Recs[i].name);
 switch(Recs[i].type)
  {
   case tChr:
     pres += os_sprintf(pres, "%c", *((char *)Recs[i].pval)&0xff);
   break;
   case tStr:
    pres += os_sprintf(pres, "%s", Recs[i].pval);
   break;
   case tUInt:
     switch(Recs[i].size)
      {
       case sizeof(uint8_t):
        pres += os_sprintf(pres, "%d", *((uint8_t *)Recs[i].pval)&0xff);
       break;
       case sizeof(uint16_t):
        pres += os_sprintf(pres, "%d", *((uint16_t *)Recs[i].pval)&0xffff);
       break;
       case sizeof(uint32_t):
        pres += os_sprintf(pres, "%d", *((uint32_t *)Recs[i].pval));
       break;
       default: pres += os_sprintf(pres, "ERROR");
      }
   break;
   case tHex:
     switch(Recs[i].size)
      {
       case sizeof(uint8_t):
        pres += os_sprintf(pres, "%02X", *((uint8_t *)Recs[i].pval)&0xff);
       break;
       case sizeof(uint16_t):
        pres += os_sprintf(pres, "%04X", *((uint16_t *)Recs[i].pval)&0xffff);
       break;
       case sizeof(uint32_t):
        pres += os_sprintf(pres, "%08X", *((uint32_t *)Recs[i].pval));
       break;
       default: pres += os_sprintf(pres, "ERROR");
      }
   break;
   case tInt:
     switch(Recs[i].size)
      {
       case sizeof(int8_t):
        pres += os_sprintf(pres, "%i", *((int8_t *)Recs[i].pval)&0xff);
       break;
       case sizeof(int16_t):
        pres += os_sprintf(pres, "%i", *((int16_t *)Recs[i].pval)&0xffff);
       break;
       case sizeof(int32_t):
        pres += os_sprintf(pres, "%i", *((int32_t *)Recs[i].pval));
       break;
       default: pres += os_sprintf(pres, "ERROR");
      }
   break;
   case tFlt:
    //pres += dtostr(pres, *((double *)Recs[i].pval), 5);
   break;
   default: pres += os_sprintf(pres, "ERROR");
  }
  pres += os_sprintf(pres, "\n\r");
  return pres-bres;
}
//---------------------------------------------------------------------------

static int ICACHE_FLASH_ATTR view(char *bres, int len)
{
 char *pres = bres;

 int i;
 for(i = 0; i < RECS; i++)
  {
   if (pres-bres+vlen(i)+8 > len) break;
   switch(Recs[i].type)
    {
     case tCmOn:
     case tCmOff:
     case tCmSave:
     case tCmLoad:
     case tCmRst:
     case tCmHlp:
     case tCmView:
     break;
     case tChr:
     case tStr:
     case tInt:
     case tUInt:
     case tFlt:
     case tHex:
      pres += display(i, pres, len);
     break;
    }
  }
 return pres-bres;
}
//---------------------------------------------------------------------------
static tres ICACHE_FLASH_ATTR assign(int i, const char *str)
{
 tres res = rOk;
 switch(Recs[i].type)
  {
   case tChr:
     *((char *)Recs[i].pval) = str[0];
   break;
   case tStr:
    {
     int j = 0;
     for(j = 0; j < Recs[i].size; j++)
      {
       if ((str[j] == '\0')||(str[j] == '\n')||(str[j] == '\r')) break;
       if (str[j] >= ' ') ((char *)Recs[i].pval)[j] = str[j];
      }
     ((char *)Recs[i].pval)[j] = '\0'; 
     res = (j < Recs[i].size)?rOk:rError;
    }
   break;
   case tUInt:
    {
     unsigned int v = atoi(str);
     switch(Recs[i].size)
      {
       case sizeof(uint8_t):
        *((uint8_t *)Recs[i].pval) = v&0xff;
       break;
       case sizeof(uint16_t):
        *((uint16_t *)Recs[i].pval) = v&0xffff;
       break;
       case sizeof(uint32_t):
        *((uint32_t *)Recs[i].pval) = v;
       break;
       default: res = rError;
      }
    }
   break;
   case tHex:
    {
     unsigned int v = htoi(str);
     switch(Recs[i].size)
      {
       case sizeof(uint8_t):
        *((uint8_t *)Recs[i].pval) = v&0xff;
       break;
       case sizeof(uint16_t):
        *((uint16_t *)Recs[i].pval) = v&0xffff;
       break;
       case sizeof(uint32_t):
        *((uint32_t *)Recs[i].pval) = v;
       break;
       default: res = rError;
      }
    }
   break;
   case tInt:
    {
     int v = atoi(str);
     switch(Recs[i].size)
      {
       case sizeof(int8_t):
        *((int8_t *)Recs[i].pval) = v&0xff;
       break;
       case sizeof(int16_t):
        *((int16_t *)Recs[i].pval) = v&0xffff;
       break;
       case sizeof(int32_t):
        *((int32_t *)Recs[i].pval) = v;
       break;
       default: res = rError;
      }
    }
   break;
   case tFlt:
    {
     if (Recs[i].size == sizeof(double))
      {
       double dbl = 0;//str2d(str);
       *((double *)Recs[i].pval) = dbl;
      }
    }
   break;
  }
 return res;
}
//---------------------------------------------------------------------------
int ICACHE_FLASH_ATTR parse(const char *str, char *bres, int len)
{
 static int bi = -1;
 static char* pbuf = 0L;
 static int bidx = 0;
 int i, j, k;
 tres res = rError;
 char c, cc;
 static enum {stParse, stB64} state = stParse;
 char *pres = bres;

 //if ((str[0]=='\0')||(str[0]<' '))

 if (!bres)
  {
   bi = -1;
   pbuf = 0L;
   bidx = 0;
   state = stParse;
   return 0;
  }

 bres[0] = '\0';
 if (state == stParse)
  {
   for(i = 0; i < RECS; i++)
    {
     j = k = 0;
     for(;;)
      {
       c = str[j++];
       if (c == ' ') continue;
       cc = Recs[i].name[k++];
       if ((c == '\0')||(cc == '\0')) break;
       if (c >= 'a' && c <= 'z') c -= ('a' - 'A');
       if (c == cc) continue;
       else break;
      }
     if ((c == '\0')||(cc == '\0')) break;
    }
   if (cc == '\0')  //found
    {
     res = rOk;
     if (c == '=')
      {
       while(str[j] == ' ') j++;
       if (Recs[i].type == tB64)
        {
         state = stB64;
         bidx = 0;
         bi = i;
         res = rBOk;
         pbuf = (char *)os_zalloc(Recs[i].size);
         if (pbuf)
          {
           int sz = b642data(&str[j], &pbuf[bidx]);
           bidx += sz;
           if (sz == 0) state = stParse;
           //PRN("b64[%d]:%s\n", bidx, &str[j]);
          }
         else res = rError;
         j = 0;
        }
       else res = assign(i, &str[j]);
      }
     else
     if (c == '?') pres += display(i, bres, len);
     else
     if ((c == '\0')||(c == '\n')||(c == '\t')||
         (c == ' ')||(c == '\r')) pres += command(i, bres, len);
     else res = rError;
    }
  }
 else
 if (state == stB64)
  {
   j = 0;
   res = rBOk;
   if (pbuf)
    {
     int sz = b642data(&str[j], &pbuf[bidx]);
     bidx += sz;
     if (sz == 0) //Key/Cert complete;
      {
       PRN("b64 done %d\n", bidx);
       state = stParse;
       if (pbuf)
        {
         if (bi >= 0) res = save_dat(bi, pbuf, bidx);
         os_free(pbuf);
        }
       bidx = 0;
      }
     //PRN("b64[%d]:%s\n", bidx, &str[j]);
    }
   else res = rError;
  }
 if (res == rOk) pres += os_sprintf(pres, "OK\n\r");
 else
 if (res == rError) pres += os_sprintf(pres, "ERROR\n\r");
 PRN("Res=%d\n", pres-bres);
 return pres-bres;
}
//---------------------------------------------------------------------------


