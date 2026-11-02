/* 延时接口：初始化后可使用微秒和毫秒等待，长等待采用毫秒分段。 */
#ifndef __DELAY_H__
#define __DELAY_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "n32g031.h"

/* 配置自由运行的24 位系统滴答计数器，并计算每微秒时钟数。 */
void Delay_Init(void);

/* 每次等待一毫秒，循环组合长延时。 */
void Delay_ms(uint32_t ms);

/* 忙等待指定微秒数；仅支持一个计数回绕以内的短延时。
 * 48MHz时单次需小于约349525 微秒，长等待使用毫秒接口。 */
void Delay_us(uint32_t us);

#ifdef __cplusplus
}
#endif

#endif
