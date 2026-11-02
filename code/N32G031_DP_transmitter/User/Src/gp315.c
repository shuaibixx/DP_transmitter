/* 电流输出驱动：定时器产生脉宽调制信号，并应用厂家四段占空比补偿。 */
#include "gp315.h"

#define GP315_PWM_STEPS     (GP315_PWM_PERIOD + 1u)

#if GP315_CAL_ENABLE
static float s_cal_low_duty;
static float s_cal_high_duty;
static uint8_t s_cal_status;
#endif
/*
* PWM输入0~25mA输出模式PWM占空比补偿函数
* 输入：0~1 （代表0.0%~100.0%）
* 输出：0~1 （补偿后的值）
*
* 使用说明：直接把你的占空比传进来，拿返回值去设PWM占空比就行
*/
double PWM_to_Current_Processor(double duty_in)
{
	double k1 = 0.0036, k2 = 0.0021, k3 = 0.0027, k4 = 0.0030;
	double duty_out = 0;
	// 防止输入超范围
	if (duty_in < 0) duty_in = 0;
	if (duty_in > 1) duty_in = 1;
	// 分成四段补偿
	if (duty_in <= 0.25)
	duty_out = (duty_in - 0) * (1 + k1);
	else if ((0.25 < duty_in) && (duty_in <= 0.50))
	duty_out = (duty_in - 0) * (1 + k1) - (duty_in - 0.25)* k2;
	else if ((0.50 < duty_in) && (duty_in <= 0.75))
	duty_out = (duty_in - 0) * (1 + k1) - (duty_in - 0.25)* k2 - (duty_in -
	0.50)* k3;
	else
	duty_out = (duty_in - 0) * (1 + k1) - (duty_in - 0.25)* k2 - (duty_in -
	0.50)* k3 - (duty_in - 0.75)* k4;
	// 防止输出超范围
	if (duty_out < 0) duty_out = 0;
	if (duty_out > 1) duty_out = 1;
	return duty_out;
}

/* 将百分比换算为定时器比较值；零和百分之百分别对应常低和常高。
 * 中间值向下取整，实际占空比存在计数分辨率误差。 */
static uint32_t gp315_percent_to_compare(float duty_percent)
{
    uint32_t compare;

    if (duty_percent <= 0.0f) return 0u;
    if (duty_percent >= 100.0f) return GP315_PWM_STEPS;
    compare = (uint32_t)(duty_percent * (float)GP315_PWM_STEPS / 100.0f);
    if (compare > GP315_PWM_STEPS) compare = GP315_PWM_STEPS;
    return compare;
}

static void gp315_write_duty(float duty_percent)
{
    TIM3->CCDAT1 = (uint16_t)gp315_percent_to_compare(duty_percent);
}

#if GP315_CAL_ENABLE && GP315_CAL_AUTO_ENABLE
/* 根据厂家补偿后的占空比反求输入；仅在初始化阶段执行。 */
static float gp315_compare_to_input(uint32_t compare)
{
    double low = 0.0;
    double high = 1.0;
    double middle;
    double fraction;
    uint8_t i;

    if (compare == 0u) return 0.0f;
    if (compare >= GP315_PWM_STEPS) return 100.0f;
    /* 取该计数区间中点，避免浮点转换后落入相邻计数。 */
    fraction = ((double)compare + 0.5) / (double)GP315_PWM_STEPS;
    for (i = 0u; i < 32u; i++)
    {
        middle = (low + high) * 0.5;
        if (PWM_to_Current_Processor(middle) < fraction) low = middle;
        else high = middle;
    }
    return (float)((low + high) * 50.0);
}

/* 与实际输出走相同的补偿、浮点转换和计数量化路径。 */
static uint32_t gp315_input_to_compare(float duty_percent)
{
    if (duty_percent <= 0.0f) return 0u;
    if (duty_percent >= 100.0f) return GP315_PWM_STEPS;
    return gp315_percent_to_compare((float)
        (PWM_to_Current_Processor((double)duty_percent / 100.0) * 100.0));
}
#endif

/* 基础端点与实测值配对；不将上次计算结果再次作为基础进行叠加。 */
static void gp315_init_calibration(void)
{
#if GP315_CAL_ENABLE
    s_cal_low_duty = GP315_CAL_4MA_DUTY_PCT;
    s_cal_high_duty = GP315_CAL_20MA_DUTY_PCT;
    s_cal_status = GP315_CAL_STATUS_BASE;
#if GP315_CAL_AUTO_ENABLE
    {
        double measured_low = (double)GP315_CAL_MEASURED_LOW_MA;
        double measured_high = (double)GP315_CAL_MEASURED_HIGH_MA;
        uint32_t low_compare;
        uint32_t high_compare;
        uint32_t target_low;
        uint32_t target_high;
        double low_result;
        double high_result;
        double slope;
        float low_input;
        float high_input;

        /* 任意实测宏为0表示未填写，直接使用基础端点，不进入计算。 */
        if ((measured_low == 0.0f)
            || (measured_high == 0.0f)) return;
        /* 非零数据顺序错误或基础占空比非法时，报告错误并保持基础端点。 */
        s_cal_status = GP315_CAL_STATUS_INVALID;
        if (!(measured_low > 0.0f)
            || !(measured_high > measured_low)
            || !(measured_high <= GP315_CURRENT_MAX_MA)
            || !(s_cal_low_duty >= 0.0f)
            || !(s_cal_high_duty > s_cal_low_duty)
            || !(s_cal_high_duty <= 100.0f)) return;

        low_compare = gp315_input_to_compare(s_cal_low_duty);
        high_compare = gp315_input_to_compare(s_cal_high_duty);
        if (high_compare <= low_compare) return;
        slope = (double)(high_compare - low_compare)
              / ((double)measured_high
                 - (double)measured_low);
        low_result = (double)low_compare
                   + ((double)GP315_CAL_REF_LOW_MA
                      - (double)measured_low) * slope;
        high_result = (double)low_compare
                    + ((double)GP315_CAL_REF_HIGH_MA
                       - (double)measured_low) * slope;
        /* 修正超出PWM范围时提示错误，不静默限幅成一个错误的标定结果。 */
        if (!(low_result >= 0.0) || !(high_result > low_result)
            || !(high_result <= (double)GP315_PWM_STEPS)) return;
        target_low = (uint32_t)(low_result + 0.5);
        target_high = (uint32_t)(high_result + 0.5);
        if (target_high <= target_low) return;
        low_input = gp315_compare_to_input(target_low);
        high_input = gp315_compare_to_input(target_high);
        /* 校验反算结果经原输出路径后确实落在目标计数。 */
        if (gp315_input_to_compare(low_input) != target_low
            || gp315_input_to_compare(high_input) != target_high) return;
        s_cal_low_duty = low_input;
        s_cal_high_duty = high_input;
        s_cal_status = GP315_CAL_STATUS_APPLIED;
    }
#endif
#endif
}

/* 返回本次上电实际使用的端点；状态区分基础值、自动修正及无效数据。 */
uint8_t GP315_GetCalibration(float *low_duty, float *high_duty)
{
#if GP315_CAL_ENABLE
    if (low_duty != 0) *low_duty = s_cal_low_duty;
    if (high_duty != 0) *high_duty = s_cal_high_duty;
    return s_cal_status;
#else
    if (low_duty != 0) *low_duty = GP315_DUTY_MIN_PCT;
    if (high_duty != 0) *high_duty = GP315_DUTY_MAX_PCT;
    return GP315_CAL_STATUS_BASE;
#endif
}

/* 配置输出引脚和定时器，初始占空比为零。 */
void GP315_Init(void)
{
    GPIO_InitType        gpio;
    TIM_TimeBaseInitType time_base;
    OCInitType           output_compare;

    RCC_EnableAPB1PeriphClk(RCC_APB1_PERIPH_TIM3, ENABLE);
    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOA | RCC_APB2_PERIPH_AFIO, ENABLE);

    GPIO_InitStruct(&gpio);
    gpio.Pin            = GP315_PWM_PIN;
    gpio.GPIO_Mode      = GPIO_MODE_AF_PP;
    gpio.GPIO_Pull      = GPIO_NO_PULL;
    gpio.GPIO_Speed     = GPIO_SPEED_HIGH;
    gpio.GPIO_Current   = GPIO_DC_LOW;
    gpio.GPIO_Alternate = GPIO_AF2_TIM3;
    GPIO_InitPeripheral(GP315_PWM_PORT, &gpio);

    TIM_InitTimBaseStruct(&time_base);
    time_base.Period    = (uint16_t)GP315_PWM_PERIOD;
    time_base.Prescaler = (uint16_t)GP315_TIM_PRESCALER;
    time_base.ClkDiv    = 0u;
    time_base.CntMode   = TIM_CNT_MODE_UP;
    TIM_InitTimeBase(TIM3, &time_base);

    TIM_InitOcStruct(&output_compare);
    output_compare.OcMode      = TIM_OCMODE_PWM1;
    output_compare.OutputState = TIM_OUTPUT_STATE_ENABLE;
    output_compare.Pulse       = 0u;
    output_compare.OcPolarity  = TIM_OC_POLARITY_HIGH;
    TIM_InitOc1(TIM3, &output_compare);

    TIM_ConfigOc1Preload(TIM3, TIM_OC_PRE_LOAD_ENABLE);
    TIM_ConfigArPreload(TIM3, ENABLE);

    TIM_Enable(TIM3, ENABLE);

    gp315_init_calibration();
    gp315_write_duty(0.0f);
}

/* 对目标占空比应用厂家四段补偿，再更新定时器。 */
void GP315_SetOutputPercent(float duty_percent)
{
    double fraction;

    if (duty_percent <= 0.0f)
    {
        gp315_write_duty(0.0f);
        return;
    }
    if (duty_percent >= 100.0f)
    {
        gp315_write_duty(100.0f);
        return;
    }

    fraction = PWM_to_Current_Processor((double)duty_percent / 100.0);
    gp315_write_duty((float)(fraction * 100.0));
}

/* 由目标电流在两点标定占空比间插值，再调用厂家原函数；不改写目标电流。 */
void GP315_SetCurrent_mA(float milliamps)
{
    float duty;

#if GP315_CAL_ENABLE
    /* 4～20mA区间按占空比端点插值，区间外线性延伸，由输出接口限幅到0～100%。 */
    duty = (float)((double)s_cal_low_duty
         + ((double)milliamps - (double)GP315_CAL_REF_LOW_MA)
         / ((double)GP315_CAL_REF_HIGH_MA - (double)GP315_CAL_REF_LOW_MA)
         * ((double)s_cal_high_duty - (double)s_cal_low_duty));
#else

    if (milliamps <= GP315_CURRENT_MIN_MA)
    {
        duty = GP315_DUTY_MIN_PCT;
    }
    else if (milliamps >= GP315_CURRENT_MAX_MA)
    {
        duty = GP315_DUTY_MAX_PCT;
    }
    else
    {
        duty = (milliamps - GP315_CURRENT_MIN_MA)
             / (GP315_CURRENT_MAX_MA - GP315_CURRENT_MIN_MA)
             * (GP315_DUTY_MAX_PCT - GP315_DUTY_MIN_PCT)
             + GP315_DUTY_MIN_PCT;
    }

#endif
    GP315_SetOutputPercent(duty);
}

