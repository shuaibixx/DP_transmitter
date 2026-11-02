/* GP315驱动配置与接口；main.h仅提供常改的实测电流和标定目标。 */
#ifndef __GP315_H__
#define __GP315_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/* 输出连接主控 PA4，再接电流转换芯片的脉宽调制输入。 */
#define GP315_PWM_PORT          GPIOA
#define GP315_PWM_PIN           GPIO_PIN_4

/* 应用保持 APB1/APB2 不分频，定时器直接使用内核时钟计数，不再预分频。 */
#define GP315_TIMER_CLK_HZ      (SystemCoreClock)
#define GP315_PWM_TICK_HZ       GP315_TIMER_CLK_HZ

/* 输出频率单位赫兹，默认一千；8MHz 时每周期8000计数，占空比每步0.0125个百分点。 */
#define GP315_PWM_FREQ_HZ       (1000u)

#define GP315_TIM_PRESCALER     ((GP315_TIMER_CLK_HZ / GP315_PWM_TICK_HZ) - 1u)
#define GP315_PWM_PERIOD        ((GP315_PWM_TICK_HZ / GP315_PWM_FREQ_HZ) - 1u)

/* 按 0～25mA 对应 0%～100% 换算，作为外部万用表标定前的初值。
 * 毫安接口可先做板级两点校准，随后映射并调用厂家四段占空比补偿。 */
#define GP315_DUTY_MIN_PCT      (0.0f)
#define GP315_DUTY_MAX_PCT      (100.0f)
#define GP315_CURRENT_MIN_MA    (0.0f)
#define GP315_CURRENT_MAX_MA    (25.0f)

/* 两个基础占空比必须是采集main.h中实测数据时使用的值，单位百分数。
 * 不能把自动算出的修正端点填回这里而继续使用旧实测值。 */
#define GP315_CAL_ENABLE          (1)
#define GP315_CAL_REF_LOW_MA      (4.0f)
#define GP315_CAL_REF_HIGH_MA     (20.0f)
#define GP315_CAL_4MA_DUTY_PCT    (15.867984f)
#define GP315_CAL_20MA_DUTY_PCT   (79.875984f)

/* 0：直接使用基础端点，完全不做自动计算，供采集基准数据。
 * 1：初始化时根据实测电流计算修正端点，以后输出直接使用缓存结果。
 * 更换板子后先设0，固定输出4/20mA测量，再填写实测值并设1。
 * 任意实测值为0时跳过计算；非零但顺序错误或超范围时报告无效。 */
#define GP315_CAL_AUTO_ENABLE      (1)

#define GP315_CAL_STATUS_BASE      (0u)
#define GP315_CAL_STATUS_APPLIED   (1u)
#define GP315_CAL_STATUS_INVALID   (2u)

/* 初始化后读取实际使用的补偿前端点及状态，指针允许为空。 */
uint8_t GP315_GetCalibration(float *low_duty, float *high_duty);

/* 配置输出引脚和定时器，初始占空比为零。 */
void GP315_Init(void);

/* 对目标占空比应用厂家四段补偿，再更新定时器。 */
void GP315_SetOutputPercent(float duty_percent);

/* 按标定的两点占空比插值，再调用厂家原补偿函数。 */
void GP315_SetCurrent_mA(float milliamps);

#ifdef __cplusplus
}
#endif

#endif
