/*
 * datastr.h
 *
 *  Created on: Feb 1, 2018
 *      Author: Dmitry Orlov
 */

#ifndef MODULES_INCLUDE_DATASTR_H_
#define MODULES_INCLUDE_DATASTR_H_

//The base64 ratio of output bytes to input bytes is 4:3 (33% overhead).
typedef unsigned char (tdatafn)(int idx);


extern bool ICACHE_FLASH_ATTR hex2dat(const char* xstr, int data_len, unsigned char* data);
extern int ICACHE_FLASH_ATTR dat2hex(char *str, unsigned char *data, int size);
extern int ICACHE_FLASH_ATTR dat2b64(char *str, unsigned char *data, int size);
extern int ICACHE_FLASH_ATTR b642data(const char *src, unsigned char *dec);


#endif /* MODULES_INCLUDE_DATASTR_H_ */
