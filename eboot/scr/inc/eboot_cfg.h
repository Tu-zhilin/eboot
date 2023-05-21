/*
 * 模块名:
 * 代码描述:
 * 作者:
 * 创建时间:2023/05/11 23:18:43
 */

#ifndef __EBOOT_CFG_H
#define __EBOOT_CFG_H

#ifdef WIN32
#include <Windows.h>
#endif

#define EBOOT_DEBUG              // 调试开关
#define EBOOT_USE_FACTORY_PART   // 工厂备份区
#define EBOOT_UPDATE_VERSION_CMP // 升级版本号对比

/* 根据平台适配 */
#ifdef EBOOT_DEBUG
#define EBOOT_LOG(...)  \
    printf("[eboot]:"); \
    printf(__VA_ARGS__)
#endif

#define EBOOT_DELAY Sleep

/* 程序区名称 */
#define EBOOT_APP_PART_NAME "app"
#define EBOOT_DOWNLOAD_PART_NAME "download"
#define EBOOT_FACTORY_PART_NAME "factory"

/* eboot参数 */
#define EBOOT_VERSION "v0.0.1"
#define EBOOT_BUFFER_MAX_SIZE 256
#define EBOOT_PRODUCT_CODE 0x12345678
#define EBOOT_MAGIC_WORD 0x12345678
#define EBOOT_CRC_WORD 0x0000

#endif
