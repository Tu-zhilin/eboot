#include "eboot_type.h"
#include "eboot_cfg.h"

uint8 *calc_name = "checksum";

uint16 eboot_calc(uint16 crc, uint8 value)
{
    return (uint16)(crc += value);
}

uint8 *eboot_get_calc_name()
{
    return calc_name;
}