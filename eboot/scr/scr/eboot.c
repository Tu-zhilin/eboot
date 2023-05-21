#include "eboot.h"
#include "eboot_cfg.h"
#include "eboot_type.h"
#include "fal.h"

uint8 eboot_buffer[EBOOT_BUFFER_MAX_SIZE] = {0};

/************************** port function **************************/
extern void eboot_jump2app(uint32 app_addr);
extern void eboot_reset();
extern uint16 eboot_calc(uint16 crc, uint8 value);
extern uint8 *eboot_get_calc_name();
extern int32 eboot_version_cmp(uint32 dest, uint32 scr);
/*******************************************************************/

/// @brief eboot信息打印
void eboot_info()
{
    EBOOT_LOG(" Information：\r\n");
    EBOOT_LOG(" version %s\r\n", EBOOT_VERSION);
    EBOOT_LOG(" product code %04x\r\n", EBOOT_PRODUCT_CODE);
    EBOOT_LOG(" calc type:%s\r\n", eboot_get_calc_name());
}

/// @brief 获取代码段起始地址
/// @param name
/// @return
int32 eboot_code_start_addr(uint8 *name)
{
    const struct fal_partition *partition = fal_partition_find((const char *)name);

    if (partition == NULL)
    {
        EBOOT_LOG("can not find part %s\r\n", name);
        return -1;
    }

    return partition->offset + sizeof(struct eboot_app_hdr);
}

/// @brief 检查程序头信息有效性
/// @param app_hdr
/// @return
int32 eboot_fw_hdr_check(struct eboot_app_hdr *app_hdr)
{
    uint16 crc = EBOOT_CRC_WORD;

    /* 检查魔幻字 */
    if (app_hdr->magic_word != EBOOT_MAGIC_WORD)
    {
        EBOOT_LOG("magic check error\r\n");
        return -1;
    }
    /* 程序头校验 */
    for (uint8 i = 0; i < sizeof(struct eboot_app_hdr) - 2; i++)
    {
        crc = eboot_calc(crc, *((uint8 *)app_hdr + i));
    }
    if (app_hdr->hdr_crc != crc)
    {
        EBOOT_LOG("header crc check error\r\n");
        return -1;
    }
    /* 检查产品码 */
    if (app_hdr->product_code != EBOOT_PRODUCT_CODE)
    {
        EBOOT_LOG("product code check error\r\n");
        return -1;
    }
    return 0;
}

/// @brief 检查程序有效性
/// @param partition
/// @param app_hdr
/// @return
int32 eboot_fw_code_check(const struct fal_partition *partition, struct eboot_app_hdr *app_hdr)
{
    uint8 buffer[EBOOT_BUFFER_MAX_SIZE] = {0};
    uint32 temp_size = 0;
    uint32 crc = EBOOT_CRC_WORD;
    uint32 code_size = app_hdr->code_size;
    uint32 addr = sizeof(struct eboot_app_hdr);

    /* 检查程序大小,若描述信息超出区的设定长度,则返回错误 */
    if (app_hdr->code_size > partition->len - sizeof(struct eboot_app_hdr))
    {
        EBOOT_LOG("code size oversize %04x -> %04x\r\n", app_hdr->code_size, partition->len);
        return -1;
    }
    /* 程序校验 */
    while (code_size > 0)
    {
        temp_size = code_size > EBOOT_BUFFER_MAX_SIZE ? EBOOT_BUFFER_MAX_SIZE : code_size;
        if (fal_partition_read(partition, addr, (uint8 *)buffer, temp_size) < 0)
        {
            EBOOT_LOG("can not read data addr:%04x size:%d\r\n", addr, temp_size);
            return -1;
        }
        for (uint32 i = 0; i < temp_size; i++)
        {
            /* 通过加载不同的校验文件实现替换 */
            crc = eboot_calc(crc, buffer[i]);
        }
        code_size -= temp_size;
        addr += temp_size;
    }
    if (app_hdr->p_crc != crc)
    {
        EBOOT_LOG("code crc check error\r\n");
        return -1;
    }
    return 0;
}

/// @brief 程序版本号比对
/// @param dest_name
/// @param scr_name
/// @return
int32 eboot_fw_version_cmp(uint8 *dest_name, uint8 *scr_name)
{
    struct eboot_app_hdr scr_hdr = {0};
    struct eboot_app_hdr dest_hdr = {0};
    const struct fal_partition *scr_partition = fal_partition_find((const char *)scr_name);
    const struct fal_partition *dest_partition = fal_partition_find((const char *)dest_name);

    if (fal_partition_read(scr_partition, 0, (uint8 *)&scr_hdr, sizeof(struct eboot_app_hdr)) < 0)
    {
        EBOOT_LOG("can not read header info\r\n");
        return -1;
    }
    if (fal_partition_read(dest_partition, 0, (uint8 *)&dest_hdr, sizeof(struct eboot_app_hdr)) < 0)
    {
        return 0;
    }
    if (eboot_version_cmp(dest_hdr.version, scr_hdr.version) < 0)
    {
        return -1;
    }
    return 0;
}

/// @brief 固件有效性检查
/// @param name
/// @return
int32 eboot_fw_valid_check(uint8 *name)
{
    struct eboot_app_hdr app_hdr = {0};
    const struct fal_partition *partition = fal_partition_find((const char *)name);

    if (partition == NULL)
    {
        EBOOT_LOG("can not find %s part\r\n", name);
        return -1;
    }
    if (fal_partition_read(partition, 0, (uint8 *)&app_hdr, sizeof(struct eboot_app_hdr)) < 0)
    {
        EBOOT_LOG("can not read %s header info\r\n", name);
        return -1;
    }
    /* 程序头信息检查 */
    if (eboot_fw_hdr_check(&app_hdr) > 0)
    {
        EBOOT_LOG("%s header check error\r\n", name);
        return -1;
    }
    if (eboot_fw_code_check(partition, &app_hdr) < 0)
    {
        EBOOT_LOG("%s code check error\r\n", name);
        return -1;
    }
    return 0;
}

/// @brief 程序搬运
/// @param dest_name
/// @param scr_name
/// @return
int32 eboot_fw_move(uint8 *dest_name, uint8 *scr_name)
{
    uint32 addr, len, temp_len;
    const struct fal_partition *scr_partition = fal_partition_find((const char *)scr_name);
    const struct fal_partition *dest_partition = fal_partition_find((const char *)dest_name);

    if (scr_partition == NULL || dest_partition == NULL)
    {
        EBOOT_LOG("can not find part\r\n");
        return -1;
    }

    if (scr_partition->len > dest_partition->len)
    {
        EBOOT_LOG("%s len > %s len\r\n", scr_name, dest_name);
        return -1;
    }

    addr = 0;
    len = scr_partition->len;

    if (fal_partition_erase(dest_partition, addr, len) < 0)
    {
        EBOOT_LOG("erase error 0x%x %d\r\n", addr, len);
        return -1;
    }

    while (len > 0)
    {
        temp_len = len > EBOOT_BUFFER_MAX_SIZE ? EBOOT_BUFFER_MAX_SIZE : len;
        if (fal_partition_read(scr_partition, addr, eboot_buffer, temp_len) < 0)
        {
            EBOOT_LOG("read error 0x%x %d\r\n", addr, temp_len);
            return -1;
        }

        if (fal_partition_write(dest_partition, addr, eboot_buffer, temp_len) < 0)
        {
            EBOOT_LOG("write error 0x%x %d\r\n", addr, temp_len);
            return -1;
        }

        addr += temp_len;
        len -= temp_len;
    }

    return 0;
}

/// @brief 固件升级
/// @return
int32 eboot_fw_updata()
{
    /* 程序有效性校验 */
    if (!eboot_fw_valid_check(EBOOT_DOWNLOAD_PART_NAME))
    {
        EBOOT_LOG("%s partition data check error\r\n", EBOOT_DOWNLOAD_PART_NAME);
        return -1;
    }
    /* 版本号对比 */
#ifdef EBOOT_UPDATE_VERSION_CMP
    if (eboot_fw_version_cmp(EBOOT_APP_PART_NAME, EBOOT_DOWNLOAD_PART_NAME) < 0)
    {
        return -1;
    }
#endif
    /* 本地升级 */
    eboot_fw_move(EBOOT_APP_PART_NAME, EBOOT_DOWNLOAD_PART_NAME);

    /* TODO：释放升级程序 */
    return 0;
}

#ifdef EBOOT_USE_FACTORY_PART
/// @brief 恢复出厂程序
/// @return
int32 eboot_fw_factory()
{
    /* 程序有效性校验 */
    if (eboot_fw_valid_check((uint8 *)EBOOT_FACTORY_PART_NAME) < 0)
    {
        return -1;
    }
    /* 恢复出厂不用校验 */
    return eboot_fw_move((uint8 *)EBOOT_APP_PART_NAME, (uint8 *)EBOOT_FACTORY_PART_NAME);
}
#endif

/// @brief eboot初始化
/// @return
int32 eboot_init()
{
    if (fal_init() < 0)
    {
        return -1;
    }

    return 0;
}

/// @brief eboot启动函数
/// @param arg
void eboot_start(void *arg)
{
    int32 app_addr = 0;

    eboot_info();

    /* 初始化 */
    if (eboot_init() < 0)
    {
        EBOOT_LOG("eboot init failed\r\n");
        goto _exit;
    }
    /* 尝试本地升级 */
    eboot_fw_updata();

    /* 跳转至app */
    if ((app_addr = eboot_code_start_addr(EBOOT_APP_PART_NAME)) < 0)
    {
        goto _exit;
    }
    eboot_jump2app(app_addr);

#ifdef EBOOT_USE_FACTORY_PART
    /* 恢复出厂程序 */
    if (eboot_fw_factory() >= 0)
    {
        eboot_jump2app(app_addr);
    }
#endif

_exit:
    /* 复位 */
    EBOOT_LOG("program reset\r\n");
    EBOOT_DELAY(5000);
    eboot_reset();
}
