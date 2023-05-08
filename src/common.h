/*
 * @Author: Danny Yu
 * @Copyright: 2022 Shenzhen Qualmesh Technology Co., Ltd.
 * @Date: 2022-06-15 19:41:56
 * @Description: 
 */
#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdio.h>

#define QMESH_SDK_DEBUG     0

#define LOGE(fmt,...) dbg_printf(fmt, ##__VA_ARGS__)
#define LOGD(fmt,...) dbg_printf(fmt, ##__VA_ARGS__)

unsigned int Utils_Crc32(unsigned int crc, const void* buf, int size);
unsigned short Crc16_Ccitt(unsigned char *s8Buf, int s32Len);
void HexStringToBytes(const char *hex, unsigned char* bytes);
void BytesToHexString(const unsigned char* bytes, const int len, char *str);
void dbg_printf(const char *fmt, ...);
void debug_printf_array(unsigned char *buf, int len);

#endif /* End of #ifndef __COMMON_H__ */