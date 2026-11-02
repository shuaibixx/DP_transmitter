/* 阻塞延时：使用系统滴答定时器自由运行计数，不启用计数中断。 */
#include "delay.h"

#define SYSTICK_RELOAD  0x00FFFFFFu
#define SYSTICK_SPAN    (SYSTICK_RELOAD + 1u)

static uint32_t s_ticks_per_us = 48u;

/* 配置自由运行的24 位系统滴答计数器，并计算每微秒时钟数。 */
void Delay_Init(void)
{
    SysTick->CTRL = 0u;
    SysTick->LOAD = SYSTICK_RELOAD;
    SysTick->VAL  = 0u;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;

    s_ticks_per_us = SystemCoreClock / 1000000u;
    if (s_ticks_per_us == 0u)
    {
        s_ticks_per_us = 1u;
    }
}

/* 忙等待指定微秒数；仅支持一个计数回绕以内的短延时。
 * 48MHz时单次需小于约349525 微秒，长等待使用毫秒接口。 */
void Delay_us(uint32_t us)
{
    uint32_t start;
    uint32_t need;
    uint32_t elapsed = 0u;

    if (us == 0u)
    {
        return;
    }

    need  = us * s_ticks_per_us;
    start = SysTick->VAL;

    while (elapsed < need)
    {
        uint32_t now = SysTick->VAL;

        if (start >= now)
        {
            elapsed = start - now;
        }
        else
        {
            elapsed = start + SYSTICK_SPAN - now;
        }
    }
}

/* 每次等待一毫秒，循环组合长延时。 */
void Delay_ms(uint32_t ms)
{
    while (ms-- != 0u)
    {
        Delay_us(1000u);
    }
}
