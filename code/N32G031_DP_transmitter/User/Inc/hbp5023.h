/* 数字差压传感器接口：引脚、寄存器、状态码及采样数据定义。 */
#ifndef __HBP5023_H__
#define __HBP5023_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/* 传感器时钟接PF1，数据接 PF0，使用软件总线。 */
#define HBP5023_SCL_PORT        GPIOF
#define HBP5023_SCL_PIN         GPIO_PIN_1
#define HBP5023_SDA_PORT        GPIOF
#define HBP5023_SDA_PIN         GPIO_PIN_0

/* 表三与总线实测确认七位地址为此值；写地址左移一位，读地址再置最低位。 */
#define HBP5023_ADDR_7BIT       0x6Du
#define HBP5023_ADDR_WRITE      ((HBP5023_ADDR_7BIT << 1) | 0x00u)
#define HBP5023_ADDR_READ       ((HBP5023_ADDR_7BIT << 1) | 0x01u)

/* 寄存器定义：配置寄存器、采集控制寄存器和连续五字节温压结果。 */
#define HBP5023_REG_CFG         0xA5u
#define HBP5023_REG_SCO         0x30u
#define HBP5023_REG_PRESS_H     0x06u
#define HBP5023_REG_PRESS_M     0x07u
#define HBP5023_REG_PRESS_L     0x08u
#define HBP5023_REG_TEMP_H      0x09u
#define HBP5023_REG_TEMP_L      0x0Au

/* 配置寄存器 bit1按手册清零；这里与采集控制寄存器的位定义不同。 */
#define HBP5023_CFG_CLR_MASK    0xFDu
#define HBP5023_CMD_START       0x0Au

/* 表四已确认采集状态位为 bit3，掩码为 0x08。
 * 启动写入 0x0A；完成后读到 0x02 时 bit3 为 0，表示采集结束。 */
#define HBP5023_SCO_BUSY        0x08u

/* 跨度来源统一使用应用配置，不在驱动中重复填写。 */
#define HBP5023_FULLSCALE_SPAN_PA   SENSOR_PRESSURE_SPAN_PA

#define HBP5023_POLL_INTERVAL_MS    1u

/* 轮询超时后采用手册允许的固定延时读取，并设置诊断标记。 */
#define HBP5023_SCO_POLL_TIMEOUT_MS 50u
#define HBP5023_CONVERSION_SETTLE_MS 10u

/* 单次采样：温压物理量和采集状态提示。 */
typedef struct
{
    int32_t  press_adc; /* 器件压力寄存器输出的24位补码，经符号扩展，不作应用校正 */
    uint8_t  sco_stuck; /* 轮询超时并使用延时读取时置一 */
    float    pressure_kpa; /* 差压值，单位千帕 */
    float    temperature_c; /* 温度值，单位摄氏度 */
} HBP5023_Data;

/* 读取结果：成功或总线失败。 */
typedef enum
{
    HBP5023_OK = 0,
    HBP5023_ERR_I2C
} HBP5023_Result;

/* 初始化传感器总线，并根据出厂跨度向上取二的幂，计算压力换算系数。
 * 当前跨度20000Pa对应满刻度32768Pa，原始计数除以256得到帕。 */
void HBP5023_Init(void);

/* 配置并启动一次温压采集，等待完成后读取五字节，解析有符号数据并换算。
 * 成功仅表示传输和解码完成，不代表经过标准压力源标定。 */
HBP5023_Result HBP5023_Read(HBP5023_Data *out);

#ifdef __cplusplus
}
#endif

#endif
