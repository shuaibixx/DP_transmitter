/* 显示屏接口：分辨率为128×64，时钟接PA6，数据接PA7。 */
#ifndef __OLED_H__
#define __OLED_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "n32g031.h"

#define OLED_SCL_PORT       GPIOA
#define OLED_SCL_PIN        GPIO_PIN_6
#define OLED_SDA_PORT       GPIOA
#define OLED_SDA_PIN        GPIO_PIN_7

#define OLED_WIDTH          128u
#define OLED_HEIGHT         64u
#define OLED_PAGES          (OLED_HEIGHT / 8u)

#define OLED_I2C_ADDR       0x78u
#define OLED_CTRL_CMD       0x00u
#define OLED_CTRL_DATA      0x40u

#define OLED_FONT_6X8       8u
#define OLED_FONT_8X16      16u

/* 配置显示总线并等待上电稳定，发送商家初始化序列后开启显示。 */
void OLED_Init(void);

/* 只清除主控显存，必须刷新后屏幕才更新。 */
void OLED_Clear(void);

/* 逐页设置地址并发送整屏显存，一次刷新包含八页数据。 */
void OLED_Refresh(void);

/* 按像素坐标设置或清除显存位；超出屏幕边界时忽略。 */
void OLED_DrawPoint(uint8_t x, uint8_t y, uint8_t on);

/* 按字库绘制字符及背景，字库范围外字符用空格替代。 */
void OLED_ShowChar(uint8_t x, uint8_t y, char ch, uint8_t size, uint8_t mode);

/* 绘制可打印字符直到字符串结束；只修改显存，不直接刷新。 */
void OLED_ShowString(uint8_t x, uint8_t y, const char *str, uint8_t size, uint8_t mode);

/* 四舍五入后绘制浮点数，小数最多六位；可变长度输出不会自动清除旧字符尾部。 */
void OLED_ShowFloat(uint8_t x, uint8_t y, float value, uint8_t frac_digits, uint8_t size);

#ifdef __cplusplus
}
#endif

#endif
