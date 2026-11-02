/* 处理器异常入口：保留默认异常函数，硬件故障时停留以便调试。 */
#include "n32g031_it.h"

/* 不可屏蔽中断入口，当前未添加处理。 */
void NMI_Handler(void)
{
}

/* 硬件故障入口，循环停留以便调试。 */
void HardFault_Handler(void)
{
    while (1)
    {
    }
}

/* 系统服务异常入口，当前未使用。 */
void SVC_Handler(void)
{
}

/* 挂起系统服务异常入口，当前未使用。 */
void PendSV_Handler(void)
{
}

/* 系统滴答中断入口；延时采用轮询，因此此处无需处理。 */
void SysTick_Handler(void)
{
}
