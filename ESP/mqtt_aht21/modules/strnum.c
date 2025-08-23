//---------------------------------------------------------------------------
#ifdef _Windows
#include <string.h>
#include "esp8266.h"
#pragma hdrstop
#pragma warn -8060
#pragma package(smart_init)
#else
#include <osapi.h>
#include <os_type.h>
#include <c_types.h>
#endif
#include "strnum.h"

//---------------------------------------------------------------------------
char ICACHE_FLASH_ATTR upcase(char c)
{
 return (c >= 'a' && c <= 'z')?(c - '\32'):c;
}

//---------------------------------------------------------------------------
int ICACHE_FLASH_ATTR strLen(char *str)
{
 int i=0;
 while(str[i]) i++;
 return i;
}
//---------------------------------------------------------------------------
//delete n chars from p position of the string
int ICACHE_FLASH_ATTR dels(char *str, int p, int n)
{
 char *sstr;
 int res = 0;
 while(p--) if (!*str++) return 0;
 sstr = str;
 while(n--) if (!*++sstr) break;
 else res++;
 while(*str++ = *sstr++);
 *str = '\0';
 return res;
}
//----------------------------------
//insert char at the beginning of string
int ICACHE_FLASH_ATTR ins(char *str, char c)
{
 int l = os_strlen(str)+1;
 while(l) {str[l] = str[l-1];l--;}
 str[0] = c;
 return 1;
}
//----------------------------------
//insert char at the position p of string
int ICACHE_FLASH_ATTR pins(char *str, char c, int p)
{
 int l = os_strlen(str)+1;
 if (p < l)
  {
   while(l>p) {str[l] = str[l-1];l--;}
   str[p] = c;
   return 1;
  }
 else return 0;
}
//----------------------------------
//insert string at the beginning of string
int ICACHE_FLASH_ATTR inss(char *str, const char *ins)
{
 int ll = os_strlen(str)+1;
 int li = os_strlen(ins);
 int res = li;
 while(li)
  {
   int l = ll++;
   while(l) {str[l] = str[l-1];l--;}
   str[0] = ins[li-- -1];
  }
 return res;
}
//----------------------------------
//insert string at the the position p of string
int ICACHE_FLASH_ATTR pinss(char *str, const char *ins, int p)
{
 int ll = os_strlen(str)+1;
 int li = os_strlen(ins);
 int res = li;
 if (p < ll)
  {
   while(li)
    {
     int l = ll++;
     while(l>p) {str[l] = str[l-1];l--;}
     str[p] = ins[li-- -1];
    }
   return res;
  }
 else return 0;
}
//----------------------------------
//add char to the end of string
int ICACHE_FLASH_ATTR add(char *str, char c)
{
 while(*str) str++;
 *str++ = c;
 *str = '\0';
 return 1;
}
//----------------------------------
//add new string to the end of exist
int ICACHE_FLASH_ATTR adds(char *str, const char *add)
{
 int res=0;
 while(*str) str++;
 while ((*str++ = *add++)) res++;
 *str = '\0';
 return res;
}
//----------------------------------
//int to string
int ICACHE_FLASH_ATTR itos(char *str, int i)
{
 unsigned int u;
 int res = 0;
 str[0] = '\0';
 if (i < 0) u = -i;
 else u = i;
 if (u)
  while(u)
   {
    char c = '0'+u%10;
    res += ins(str, c);
    u /= 10;
   }
 else res += ins(str, '0');
 if (i < 0) res += ins(str, '-');
 return res;
}
//----------------------------------
//----------------------------------
//int to string
int ICACHE_FLASH_ATTR i64tos(char *str, u64 u, char p)
{
 int res = 0;
 str[0] = '\0';
 if (u)
  while(u)
   {
    char c = '0'+u%10;
    res += ins(str, c);
    if (res == p) res += ins(str, '.');
    u /= 10;
   }
 else res += ins(str, '0');
 return res;
}
//----------------------------------
//int to string with adjustment
int ICACHE_FLASH_ATTR itosa(char *str, int i, int adj)
{
 int res = itos(str, i);
 if (adj > 0) while (res < adj) res += ins(str, ' ');
 else
 if (adj < 0) while (res < -adj) res += add(str, ' ');
 return res;
}
//----------------------------------
//adjust string
int ICACHE_FLASH_ATTR sadj(char *str, int adj)
{
 int res = os_strlen(str);
 if (adj > 0) while (res < adj) res += ins(str, ' ');
 else
 if (adj < 0) while (res < -adj) res += add(str, ' ');
 return res;
}
//----------------------------------
//int to hex
int ICACHE_FLASH_ATTR itoh(char *str, int i)
{
 unsigned int u;
 int res = 0;
 str[0] = '\0';
 if (i < 0) u = -i;
 else u = i;
 if (u)
  while(u)
   {
    char c = '0'+u%16;
    if (c > '9') c += ('a'-'9'-1);//39
    res += ins(str, c);
    u /= 16;
   }
 else res += ins(str, '0');
 if (i < 0) res += ins(str, '-');
 return res;
}
//----------------------------------
//int to n hex digits
int ICACHE_FLASH_ATTR itox(char *str, unsigned int u, int n)
{
 int res = 0;
 str[0] = '\0';
 while(n)
  {
   char c = '0'+u%16;
   if (c > '9') c += ('a'-'9'-1);//39
   res += ins(str, c);
   u /= 16;
   n--;
  }
 return res;
}
//----------------------------------
//floating point double to string with or without lead zero
//negative decimals == automatic precision guess
int ICACHE_FLASH_ATTR dtostrz(char * str, double d, int decimals, int zero)
{
 char *ptr = str;
 char *p = ptr;
 char *p1;
 char c;
 long intPart;
 int wlen;
 int res;

 if (d < 0)
  {
   d = -d;
   *ptr++ = '-';
  }
 if (decimals > 10) decimals = 10;
 else
 if (decimals < 0)
  {
   double r;
   for(decimals = 6, r = 1.0; decimals; decimals--)
    {
     if (d < r) break;
     r *= 10.0;
    }
  }

 // round value according the precision
 if (decimals)
  {
   double r;
   for(c = 0, r = 0.5; c < decimals; c++) r /=10.0;
   d += r;
  }

 intPart = d;
 d -= intPart;
 if (zero && !intPart) *ptr++ = '0';
 else
  {
   p = ptr;
   // convert (reverse order)
   while (intPart)
   {
     *p++ = '0' + intPart % 10;
     intPart /= 10;
   }
   // save end pos
   p1 = p;
   // reverse result
   while (p > ptr)
    {
     c = *--p;
     *p = *ptr;
     *ptr++ = c;
    }
   // restore end pos
   ptr = p1;
 }
 // decimal part
 wlen = ptr-str;
 if (decimals)
  {
   *ptr++ = '.';
   while (decimals--)
    {
     d *= 10.0;
     c = d;
     *ptr++ = '0' + c;
     d -= c;
    }
  }
 // terminating zero
 *ptr = '\0';

 res = ptr-str;
 if (zero>=0)
  while((res>wlen)&&((str[res-1]=='0')||(str[res-1]=='.'))) str[--res] = '\0';
 return res;
}
//----------------------------------
//floating point double to string
int ICACHE_FLASH_ATTR dtostr(char *str, double d, int decimals)
{
  return dtostrz(str, d, decimals, 1);
}
//----------------------------------
//return rounded value with n fract digits
double ICACHE_FLASH_ATTR rnd(double d, int n)
{
 double Pow_10n = 1.0;
 while(n--) Pow_10n *= 10.0;
 if (d < 0) return (int)(d * Pow_10n - 0.5) / Pow_10n;
 else return (int)(d * Pow_10n + 0.5) / Pow_10n;
}
//----------------------------------
//return rounded value with n significant digits
double ICACHE_FLASH_ATTR nrnd(double d, int n)
{
 int exp = 0;
 double Pow_10n = 1.0;
 double dd = (d<0)?-d:d;
 if (dd > 0)
  {
   while (dd >= 1) {dd /= 10; exp++;}
   while (dd < 1)  {dd *= 10; exp--;}
  }
 if (n) n--;
 while(n--) Pow_10n *= 10.0;
 dd = (int)((dd * Pow_10n + 0.5)) / Pow_10n;
 while(exp)
  if (exp > 0) {dd *= 10.0; exp--;}
  else {dd /= 10.0; exp++;}
 if (d < 0) return -dd;
 else return dd;
}
//---------------------------------------------------------------------------
//time in seconds to date and time string
typedef enum {centures, years, days, hours, minutes, seconds, times } ttimes;

int ICACHE_FLASH_ATTR t2str(char *str, uint64_t sec, int adj, bool full)
{
 const uint64_t dms[] =
 {(60LL*60*60*24*365.25*100LL),(60LL*60*24*365.25),
  (60LL*60*24), (60LL*60), 60LL, 1LL};
 const char * fmt[] =
   {":c ", ":y ", ":d ", ":h ", ":m ", ":s "};
 const char w[] =
   { 0,      3,     3,     2,     2,     2};
 unsigned int pt[times];
 int i, j, k;
 int res;
 char *pc = str;

 for(i = 0, j = -1, k = 0; i < times; i++)
  {
    pt[i] = (unsigned int)(sec / dms[i]);
    sec %= dms[i];
    if ((j == -1) && (pt[i] != 0)) j = i;
    if ((j != -1) && (pt[i] != 0)) k = i;
  }
 if (full) k = seconds;
 *str = '\0';
 if (j == -1) str += adds(str, "0:s");
 else
 for(i = j; i <= k; i++)
  {
   str += itosa(str, pt[i], w[i]);
   str += adds(str, fmt[i]);
  }
 res = str-pc;
 if (adj > 0) while (res < adj) res += ins(pc, ' ');
 else
 if (adj < 0) while (res < -adj) res += add(str, ' ');
 return res;
}
//---------------------------------------------------------------------------
//time in seconds to date and time string
int ICACHE_FLASH_ATTR dt2str(char *str, double dsec, int adj, bool full)
{
  const uint64_t dms[] =
  {(60LL*60*60*24*365.25*100LL),(60LL*60*24*365.25),
   (60LL*60*24), (60LL*60), 60LL, 1LL};
 const char * fmt[] =
   {":c ", ":y ", ":d ", ":h ", ":m ", ":s "};
 const char w[] =
   { 0,      3,     3,     2,     2,     2};
 unsigned int pt[times];
 int i, j, k;
 int res;
 char *pc = str;
 int sec = (int)dsec;
 double fract = (dsec - sec);

 for(i = 0, j = -1, k = 0; i < times; i++)
  {
    pt[i] = (unsigned int)(sec / dms[i]);
    sec %= dms[i];
    if ((j == -1) && (pt[i] != 0)) j = i;
    if ((j != -1) && (pt[i] != 0)) k = i;
  }
 if (full) k = seconds;
 *str = '\0';
 if (j == -1)
  {
   if (fract) str += dtostrz(str, fract, 3, 1);
   else str += adds(str, "0");
   str += adds(str, ":s");
  }
 else
 for(i = j; i <= k; i++)
  {
   str += itosa(str, pt[i], w[i]);
   if (i==seconds) str += dtostrz(str, fract, 3, 0);
   str += adds(str, fmt[i]);
  }
 res = str-pc;
 if (adj > 0) while (res < adj) res += ins(pc, ' ');
 else
 if (adj < 0) while (res < -adj) res += add(str, ' ');
 return res;
}
//----------------------------------
//floating point double to engineering string with prec significant digits
int ICACHE_FLASH_ATTR d2scistrup(char *str, double d, const char* units, int prec, int adj)
{
 const char csci[] = {'y', 'z', 'a', 'f', 'p', 'n', 'u', 'm', ' ',
                      'k', 'M', 'G', 'T', 'P', 'E', 'Z', 'Y'};
 enum {yocto, zepto, atto, femto, pico, nano, micro, milli, empty,
       kilo, mega, giga, tera, peta, exa, zetta, yotta};

 char *pc = str;
 double dd = (d<0)?-d:d;
 int rng = empty;
 int res;
 if (dd > 0)
  {
   while (dd >= 1000) {dd /= 1000; rng++;}
   while (dd < 1)     {dd *= 1000; rng--;}
  }
 if (d < 0) dd = -dd;
 if (rng == empty)
  {
   d =  nrnd(d, prec);
   res = dtostr(str, d, prec);
  }
 else
  {
   dd =  nrnd(dd, prec);
   res = dtostr(str, dd, prec);
   if ((rng >= yocto) && (rng <= yotta))
    res += add(str, csci[rng]);
   else
    {
     res += add(str, 'e');
     res += itos(str+res, (rng-empty)*3);
    }
  }
 res += adds(str, units);
 if (adj > 0) while (res < adj) res += ins(pc, ' ');
 else
 if (adj < 0) while (res < -adj) res += add(str, ' ');
 return res;
}
//---------------------------------------------------------------------------
//floating point double to engineering string
int ICACHE_FLASH_ATTR d2scistru(char *str, double d, const char* units)
{
 return d2scistrup(str, d, units, 4, 0);
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//string to double
double ICACHE_FLASH_ATTR str2d(const char *str)
{
 return str2de(str, NULL);
}
//---------------------------------------------------------------------------
//string to double
double ICACHE_FLASH_ATTR str2de(const char *str, const char **stop)
{
 double d = 0;
 double m = 0.1;
 int fract = 0;
 int i=0;
 char c;
 int sign;
 while (str[i] && (str[i] == ' ')) i++; //skip lead spaces
 if (str[i] == '-') {sign = -1; i++;}
 else
 if (str[i] == '+') {sign = 1; i++;}
 else sign = 1;
 for(;(c = str[i]); i++)
  {
   if (c == ' ') continue;
   if ((c == 'e')||(c == 'E'))
    {
     int e = 0;
     int esign;
     i++;
     while (str[i] && (str[i] == ' ')) i++; //skip lead spaces
     if (str[i] == '-') {esign = -1; i++;}
     else
     if (str[i] == '+') {esign = 1; i++;}
     else esign = 1;
     for(;(c = str[i]); i++)
      {
       if (c == ' ') continue;
       if (c >= '0' && c <= '9') e = e*10 + (c - '0');
       else break;
      }
     e *= esign;
     if (e > 0) while(e--) d*=10.0;
     else
     if (e < 0) while(e++) d*=0.1;
     break;
    }
   if (c >= '0' && c <= '9')
    {
     if (!fract) d = d*10.0 + (c - '0');
     else
      {
       d = d+(c - '0')*m;
       m /= 10.0;
      }
    }
   else
   if (c == '.') fract = 1;
   else break;
  }
 if (stop) *stop = &str[i];
 return sign*d;
}
//---------------------------------------------------------------------------
//string to int
int ICACHE_FLASH_ATTR str2i(const char *str, const char **stop)
{
 enum {tDec, tOct, tHex, tBin} t = tDec; //data type
 int d = 0;
 int i = 0;
 char c;
 int sign;
 while (str[i] && (str[i] == ' ')) i++; //skip lead spaces
 if (str[i] == '-') {sign = -1; i++;}
 else
 if (str[i] == '+') {sign = 1; i++;}
 else sign = 1;
 while (str[i] && (str[i] == ' ')) i++; //skip lead spaces
 if ((str[i] == '0') && (c = str[i+1])) //oct, hex, bin
  {
   i++;
   if (c >= '0' && c <= '7') t = tOct;
   else
   if (c == 'O'||c == 'o') {t = tOct; i++;}
   else
   if (c == 'X'||c == 'x') {t = tHex; i++;}
   else
   if (c == 'B'||c == 'b') {t = tBin; i++;}
   else t = tDec;
  }
 for(;(c = str[i]); i++)
  {
   if (c == ' ') continue; //skip spaces
   if (t == tDec)
    {
     if (c >= '0' && c <= '9') d = d*10 + (c - '0');
     else break;
    }
   else
   if (t == tOct)
    {
     if (c >= '0' && c <= '7') d = d*8 + (c - '0');
     else break;
    }
   else
   if (t == tBin)
    {
     if (c >= '0' && c <= '1') d = d*2 + (c - '0');
     else break;
    }
   else
   if (t == tHex)
    {
     if (c >= '0' && c <= '9') d = d*16 + (c - '0');
     else
     if (c >= 'A' && c <= 'F') d = d*16 + (c - 'A'+10);
     else
     if (c >= 'a' && c <= 'f') d = d*16 + (c - 'a'+10);
     else break;
    }
  }
 if (stop) *stop = &str[i];
 return sign*d;
}
//---------------------------------------------------------------------------
