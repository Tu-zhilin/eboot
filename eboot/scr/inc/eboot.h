#ifndef __EBOOT_H
#define __EBOOT_H

#include "eboot_type.h"

struct eboot_app_hdr
{
    uint32 magic_word;   /* 魔幻字 */
    uint32 version;      /* 版本号 */
    uint32 product_code; /* 产品号 */
    uint32 code_size;    /* 程序大小 */
    uint32 p_crc;        /* 程序校验码 */
    uint32 hdr_crc;      /* 头校验码 */
};

extern void eboot_start(void *arg);

#endif
