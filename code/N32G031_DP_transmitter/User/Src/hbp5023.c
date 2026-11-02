/* 数字差压传感器驱动：寄存器读写、转换等待、补码解析和温压换算。 */
#include "hbp5023.h"
#include "soft_i2c.h"
#include "delay.h"

static const SoftI2C_Bus s_hbp5023_bus =
{
    HBP5023_SCL_PORT, HBP5023_SCL_PIN,
    HBP5023_SDA_PORT, HBP5023_SDA_PIN
};

static uint8_t s_shift;

/* 初始化传感器总线，并根据出厂跨度向上取二的幂，计算压力换算系数。
 * 当前跨度20000Pa对应满刻度32768Pa，原始计数除以256得到帕。 */
void HBP5023_Init(void)
{
    uint32_t span;
    uint32_t power = 1u;
    uint8_t  bits  = 0u;

    SoftI2C_Init(&s_hbp5023_bus);

    span = (uint32_t)HBP5023_FULLSCALE_SPAN_PA;
    while ((power < span) && (bits < 31u))
    {
        power <<= 1;
        bits++;
    }

    if (bits <= 23u)
    {
        s_shift = (uint8_t)(23u - bits);
    }
    else
    {
        s_shift = 0u;
    }
}

/* 发送寄存器地址与一个数据字节；返回零表示收到应答。 */
static uint8_t hbp5023_write_reg(uint8_t reg, uint8_t value)
{
    uint8_t frame[2];

    frame[0] = reg;
    frame[1] = value;

    return SoftI2C_Write(&s_hbp5023_bus, HBP5023_ADDR_WRITE, frame, 2u);
}

/* 先写寄存器地址，再通过重复起始读取指定数量的字节。 */
static uint8_t hbp5023_read_reg(uint8_t reg, uint8_t *data, uint16_t len)
{
    return SoftI2C_WriteRead(&s_hbp5023_bus, HBP5023_ADDR_WRITE,
                             &reg, 1u, data, len);
}

/* 配置并启动一次温压采集，等待完成后读取五字节，解析有符号数据并换算。
 * 成功仅表示传输和解码完成，不代表经过标准压力源标定。 */
HBP5023_Result HBP5023_Read(HBP5023_Data *out)
{
    uint8_t  cfg;
    uint8_t  sco;
    uint8_t  raw[5];
    uint32_t waited = 0u;
    int32_t  press_adc;
    int16_t  temp_adc;
    float    pressure_pa;

    if (out == 0)
    {
        return HBP5023_ERR_I2C;
    }

    if (hbp5023_read_reg(HBP5023_REG_CFG, &cfg, 1u) != 0u)
    {
        return HBP5023_ERR_I2C;
    }

    /* 按手册清除配置寄存器 bit1后写回。 */
    cfg = (uint8_t)(cfg & HBP5023_CFG_CLR_MASK);

    if (hbp5023_write_reg(HBP5023_REG_CFG, cfg) != 0u)
    {
        return HBP5023_ERR_I2C;
    }

    if (hbp5023_write_reg(HBP5023_REG_SCO, HBP5023_CMD_START) != 0u)
    {
        return HBP5023_ERR_I2C;
    }

    /* 检查采集状态 bit3；超时后延时读取，不能将诊断标记直接当成器件损坏。 */
    out->sco_stuck = 0u;
    for (;;)
    {
        if (hbp5023_read_reg(HBP5023_REG_SCO, &sco, 1u) != 0u)
        {
            return HBP5023_ERR_I2C;
        }

        if ((sco & HBP5023_SCO_BUSY) == 0u)
        {
            break;
        }

        if (waited >= HBP5023_SCO_POLL_TIMEOUT_MS)
        {
            out->sco_stuck = 1u;
            Delay_ms(HBP5023_CONVERSION_SETTLE_MS);
            break;
        }

        Delay_ms(HBP5023_POLL_INTERVAL_MS);
        waited += HBP5023_POLL_INTERVAL_MS;
    }

    if (hbp5023_read_reg(HBP5023_REG_PRESS_H, raw, 5u) != 0u)
    {
        return HBP5023_ERR_I2C;
    }

    press_adc = ((int32_t)raw[0] << 16)
              | ((int32_t)raw[1] << 8)
              | ((int32_t)raw[2]);

    /* 将24 位补码扩展为有符号32 位，保留负压。 */
    if ((press_adc & 0x00800000) != 0)
    {
        press_adc -= 0x01000000;
    }

    temp_adc = (int16_t)(((uint16_t)raw[3] << 8) | (uint16_t)raw[4]);

    /* 压力除以换算系数得到帕，再转为千帕；温度计数除以256得到摄氏度。 */
    out->press_adc = press_adc;
    pressure_pa   = (float)press_adc / (float)(1u << s_shift);
    out->pressure_kpa  = pressure_pa / 1000.0f;
    out->temperature_c = (float)temp_adc / 256.0f;

    return HBP5023_OK;
}
