/*
 * strnum.h
 *
 *  Created on: Jan 1, 2018
 *      Author: Dmitry Orlov
 */

#ifndef INCLUDE_STRNUM_H_
#define INCLUDE_STRNUM_H_

extern char ICACHE_FLASH_ATTR upcase(char c);

//delete n chars from p position of the string
extern int ICACHE_FLASH_ATTR dels(char *str, int p, int n);

//insert char at the beginning of string
extern int ICACHE_FLASH_ATTR ins(char *str, char c);

//insert char at the position p of string
extern int ICACHE_FLASH_ATTR pins(char *str, char c, int p);

//insert string at the beginning of string
extern int ICACHE_FLASH_ATTR inss(char *str, const char *ins);

//insert string at the the position p of string
extern int ICACHE_FLASH_ATTR pinss(char *str, const char *ins, int p);

//add char to the end of string
extern int ICACHE_FLASH_ATTR add(char *str, char c);

//add new string to the end of exist
extern int ICACHE_FLASH_ATTR adds(char *str, const char *add);

//int to string
extern int ICACHE_FLASH_ATTR itos(char *str, int i);

extern int ICACHE_FLASH_ATTR i64tos(char *str, u64 u, char p);


//int to string with adjustment
extern int ICACHE_FLASH_ATTR itosa(char *str, int i, int adj);

//adjust string
extern int ICACHE_FLASH_ATTR sadj(char *str, int adj);

//int to hex
extern int ICACHE_FLASH_ATTR itoh(char *str, int i);

//int to n hex digits
extern int ICACHE_FLASH_ATTR itox(char *str, unsigned int u, int n);

//floating point double to string
extern int ICACHE_FLASH_ATTR dtostr(char *str, double d, int decimals);

//calculate round value
extern ICACHE_FLASH_ATTR double rnd(double d, int n);

//return rounded value with n significant digits
extern ICACHE_FLASH_ATTR double nrnd(double d, int n);

//time in seconds to date and time string
extern int ICACHE_FLASH_ATTR t2str(char *str, uint64_t sec, int adj, bool full);

//floating point double to engineering string
extern int ICACHE_FLASH_ATTR d2scistru(char *str, double d, const char* units);

//floating point double to engineering string with precision significant digits
extern int ICACHE_FLASH_ATTR d2scistrup(char *str, double d, const char* units, int prec, int adj);

//time in seconds to date and time string
extern int ICACHE_FLASH_ATTR dt2str(char *str, double dsec, int adj, bool full);

//string to double
extern double ICACHE_FLASH_ATTR str2d(const char *str);

//string to double
extern double ICACHE_FLASH_ATTR str2de(const char *str, const char **stop);

//string to int
extern int ICACHE_FLASH_ATTR str2i(const char *str, const char **stop);


//---------------------------------------------------------------------------
#endif
