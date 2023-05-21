/*
 * 模块名:
 * 代码描述:
 * 作者:
 * 创建时间:2023/05/21 20:41:06
 */

#ifndef __EBOOT_TYPE_H
#define __EBOOT_TYPE_H

#define int8 signed char
#define int16 signed short
#define int32 signed int
#define int64 signed long

#define uint8 unsigned char
#define uint16 unsigned short
#define uint32 unsigned int
#define uint64 unsigned long

#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif

#ifndef NULL
#define NULL (void *)0
#endif

#endif