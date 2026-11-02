/* 常改参数：传感器量程、GP315实测电流和固定标定目标。 */
#ifndef __MAIN_H__
#define __MAIN_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "n32g031.h"

/* 填写传感器实际出厂量程，单位千帕；当前确认为 0～20kPa。 */
#define SENSOR_PRESSURE_MIN_KPA (0.0f)
#define SENSOR_PRESSURE_MAX_KPA (20.0f)

/* 填写固定4/20mA时的万用表实测值；任意一项为0即跳过两点计算。 */
#define GP315_CAL_MEASURED_LOW_MA  (3.980f)
#define GP315_CAL_MEASURED_HIGH_MA (19.970f)

/* 将出厂量程跨度转换为帕，驱动据此计算原始数据换算系数。 */
#define SENSOR_PRESSURE_SPAN_PA \
    ((SENSOR_PRESSURE_MAX_KPA - SENSOR_PRESSURE_MIN_KPA) * 1000.0f)

/* 整机工作范围默认跟随传感器出厂量程。 */
#define APP_PRESSURE_MIN_KPA    SENSOR_PRESSURE_MIN_KPA
#define APP_PRESSURE_MAX_KPA    SENSOR_PRESSURE_MAX_KPA

/* 整机输出电流范围，单位毫安。 */
#define APP_CURRENT_MIN_MA      (4.0f)
#define APP_CURRENT_MAX_MA      (20.0f)

/* 0.0f：采集压力并转换为电流；正值：固定输出该电流，单位mA。
 * 标定时填4.0f或20.0f；固定模式不采集压力，不修改量程或零点。 */
#define APP_GP315_CAL_CURRENT_MA (0.0f)

/* 正常采集每轮末尾的等待时间，单位毫秒；实际周期还包括采集和打印耗时。 */
#define APP_SAMPLE_PERIOD_MS    (300u)

#ifdef __cplusplus
}
#endif

#endif
