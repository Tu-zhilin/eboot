#include "eboot_type.h"
#include "stm32f4xx.h"

typedef void (*app)(void);

void eboot_jump2app(uint32 app_addr)
{
    uint32 app_msp = (*((uint32 *)app_addr));
    uint32 app_reset = *((uint32 *)(app_addr + 4));
    /* 关闭全局中断 */
    __disable_irq();
    /* 重置RCC */
    HAL_RCC_DeInit();
    /* 重设栈地址 */
    __set_MSP(app_msp);
    /* 重设中断向量表 */
    SCB->VTOR = app_addr;
    /* 打开全局中断 */
    __enable_irq();
    /* 获取应用层复位中断地址, 跳转 */
    ((app)app_reset)();
}

void eboot_reset()
{
    /* 关闭全局中断 */
    __disable_irq();
    /* 软件复位 */
    HAL_NVIC_SystemReset();
}

int32 eboot_version_cmp(uint32 dest, uint32 scr)
{
    return 0;
}
