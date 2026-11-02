/* 显示屏驱动：初始化控制器、维护显存并绘制文字和数字。 */
#include "oled.h"
#include "soft_i2c.h"
#include "delay.h"
#include "oledfont.h"

static const SoftI2C_Bus s_oled_bus =
{
    OLED_SCL_PORT, OLED_SCL_PIN,
    OLED_SDA_PORT, OLED_SDA_PIN
};

/* 显存按列和页存储，每字节表示同一列的八个像素，最低位在该页顶部。 */
static uint8_t s_gram[OLED_WIDTH][OLED_PAGES];

/* 复用发送缓冲区，首字节为命令或数据控制字节。 */
static uint8_t s_tx[1u + OLED_WIDTH];

#define FONT_6X8_CHARS      (sizeof(asc2_0806) / sizeof(asc2_0806[0]))
#define FONT_8X16_CHARS     (sizeof(asc2_1608) / sizeof(asc2_1608[0]))

/* 发送显示屏控制字节及负载；当前忽略底层返回值，失败不会向上传递。 */
static void oled_write(const uint8_t *payload, uint16_t len)
{
    (void)SoftI2C_Write(&s_oled_bus, OLED_I2C_ADDR, payload, len);
}

/* 以命令控制字节发送一条显示控制命令。 */
static void oled_cmd(uint8_t cmd)
{
    uint8_t buf[2];

    buf[0] = OLED_CTRL_CMD;
    buf[1] = cmd;
    oled_write(buf, 2u);
}

/* 将多条命令放入共用缓冲区，合并为一次总线传输。 */
static void oled_cmds(const uint8_t *cmds, uint16_t len)
{
    uint16_t i;

    s_tx[0] = OLED_CTRL_CMD;
    for (i = 0u; i < len; i++)
    {
        s_tx[1u + i] = cmds[i];
    }
    oled_write(s_tx, (uint16_t)(len + 1u));
}

/* 配置显示总线并等待上电稳定，发送商家初始化序列后开启显示。 */
void OLED_Init(void)
{
    static const uint8_t init_seq[] =
    {
        0xAEu,
        0xFDu, 0x12u,
        0xD5u, 0xA0u,
        0xA8u, 0x3Fu,
        0xD3u, 0x00u,
        0x40u,
        0xA1u,
        0xC8u,
        0xDAu, 0x12u,
        0x81u, 0xBFu,
        0xD9u, 0x25u,
        0xDBu, 0x34u,
        0xA4u,
        0xA6u
    };

    SoftI2C_Init(&s_oled_bus);
    Delay_ms(50u);

    oled_cmds(init_seq, (uint16_t)sizeof(init_seq));

    OLED_Clear();
    oled_cmd(0xAFu);
}

/* 只清除主控显存，必须刷新后屏幕才更新。 */
void OLED_Clear(void)
{
    uint8_t  page;
    uint16_t col;

    for (page = 0u; page < OLED_PAGES; page++)
    {
        for (col = 0u; col < OLED_WIDTH; col++)
        {
            s_gram[col][page] = 0x00u;
        }
    }
}

/* 逐页设置地址并发送整屏显存，一次刷新包含八页数据。 */
void OLED_Refresh(void)
{
    uint8_t  page;
    uint16_t col;

    for (page = 0u; page < OLED_PAGES; page++)
    {
        oled_cmd((uint8_t)(0xB0u + page));
        oled_cmd(0x00u);
        oled_cmd(0x10u);

        s_tx[0] = OLED_CTRL_DATA;
        for (col = 0u; col < OLED_WIDTH; col++)
        {
            s_tx[1u + col] = s_gram[col][page];
        }
        oled_write(s_tx, (uint16_t)(OLED_WIDTH + 1u));
    }
}

/* 按像素坐标设置或清除显存位；超出屏幕边界时忽略。 */
void OLED_DrawPoint(uint8_t x, uint8_t y, uint8_t on)
{
    uint8_t page;
    uint8_t bit;

    if ((x >= OLED_WIDTH) || (y >= OLED_HEIGHT))
    {
        return;
    }

    page = (uint8_t)(y >> 3);
    bit  = (uint8_t)(1u << (y & 0x07u));

    if (on != 0u)
    {
        s_gram[x][page] |= bit;
    }
    else
    {
        s_gram[x][page] = (uint8_t)(s_gram[x][page] & (uint8_t)~bit);
    }
}

/* 按字库绘制字符及背景，字库范围外字符用空格替代。 */
void OLED_ShowChar(uint8_t x, uint8_t y, char ch, uint8_t size, uint8_t mode)
{
    uint8_t  i;
    uint8_t  col;
    uint8_t  temp;
    uint8_t  x0 = x;
    uint8_t  y0 = y;
    uint16_t index;

    if ((ch < ' ') || (ch > '~'))
    {
        ch = ' ';
    }
    index = (uint16_t)((uint8_t)ch - (uint8_t)' ');

    if (size == OLED_FONT_6X8)
    {
        if (index >= FONT_6X8_CHARS)
        {
            index = 0u;
        }
        for (i = 0u; i < 6u; i++)
        {
            temp = asc2_0806[index][i];
            for (col = 0u; col < 8u; col++)
            {
                OLED_DrawPoint(x, y, (uint8_t)((temp & 0x01u) ? mode : (uint8_t)!mode));
                temp = (uint8_t)(temp >> 1);
                y++;
            }
            x++;
            y = y0;
        }
    }
    else if (size == OLED_FONT_8X16)
    {
        if (index >= FONT_8X16_CHARS)
        {
            index = 0u;
        }
        for (i = 0u; i < 16u; i++)
        {
            temp = asc2_1608[index][i];
            for (col = 0u; col < 8u; col++)
            {
                OLED_DrawPoint(x, y, (uint8_t)((temp & 0x01u) ? mode : (uint8_t)!mode));
                temp = (uint8_t)(temp >> 1);
                y++;
            }
            x++;
            if ((uint8_t)(x - x0) == 8u)
            {
                x  = x0;
                y0 = (uint8_t)(y0 + 8u);
            }
            y = y0;
        }
    }
    else
    {
    }
}

/* 绘制可打印字符直到字符串结束；只修改显存，不直接刷新。 */
void OLED_ShowString(uint8_t x, uint8_t y, const char *str, uint8_t size, uint8_t mode)
{
    uint8_t advance;

    if (str == 0)
    {
        return;
    }

    advance = (size == OLED_FONT_6X8) ? 6u : (uint8_t)(size / 2u);

    while ((*str >= ' ') && (*str <= '~'))
    {
        OLED_ShowChar(x, y, *str, size, mode);
        x = (uint8_t)(x + advance);
        str++;
    }
}

/* 四舍五入后绘制浮点数，小数最多六位；可变长度输出不会自动清除旧字符尾部。 */
void OLED_ShowFloat(uint8_t x, uint8_t y, float value, uint8_t frac_digits, uint8_t size)
{
    char     buf[16];
    uint8_t  n = 0u;
    uint8_t  i;
    int32_t  scale = 1;
    uint32_t mag;
    char     digits[11];
    uint8_t  nd = 0u;

    if (frac_digits > 6u)
    {
        frac_digits = 6u;
    }

    for (i = 0u; i < frac_digits; i++)
    {
        scale *= 10;
    }

    if (value < 0.0f)
    {
        buf[n++] = '-';
        value = -value;
    }

    mag = (uint32_t)(int32_t)((value * (float)scale) + 0.5f);

    if (mag == 0u)
    {
        digits[nd++] = '0';
    }
    while (mag != 0u)
    {
        digits[nd++] = (char)('0' + (mag % 10u));
        mag /= 10u;
    }
    while (nd <= frac_digits)
    {
        digits[nd++] = '0';
    }

    for (i = nd; i > frac_digits; i--)
    {
        buf[n++] = digits[i - 1u];
    }

    if (frac_digits > 0u)
    {
        buf[n++] = '.';
        for (i = frac_digits; i > 0u; i--)
        {
            buf[n++] = digits[i - 1u];
        }
    }

    buf[n] = '\0';

    OLED_ShowString(x, y, buf, size, 1u);
}
