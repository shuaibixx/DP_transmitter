/* 处理器异常函数声明，与启动文件中的异常向量对应。 */
#ifndef __N32G031_IT_H__
#define __N32G031_IT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "n32g031.h"

/* 不可屏蔽中断入口，当前未添加处理。 */
void NMI_Handler(void);

/* 硬件故障入口，循环停留以便调试。 */
void HardFault_Handler(void);

/* 系统服务异常入口，当前未使用。 */
void SVC_Handler(void);

/* 挂起系统服务异常入口，当前未使用。 */
void PendSV_Handler(void);

/* 系统滴答中断入口；延时采用轮询，因此此处无需处理。 */
void SysTick_Handler(void);

#ifdef __cplusplus
}
#endif

#endif
