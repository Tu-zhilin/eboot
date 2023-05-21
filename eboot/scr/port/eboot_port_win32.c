#include "eboot_type.h"

typedef void (*app)(void);

void eboot_jump2app(uint32 app_addr)
{
}

void eboot_reset()
{
}

int32 eboot_version_cmp(uint32 dest, uint32 scr)
{
    return 0;
}
