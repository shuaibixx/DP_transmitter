/* 软件两线总线接口：通过总线结构体选择引脚，传输地址使用包含方向位的八位格式。 */
#ifndef __SOFT_I2C_H__
#define __SOFT_I2C_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "n32g031.h"

typedef struct
{
    GPIO_Module *scl_port;
    uint16_t     scl_pin;
    GPIO_Module *sda_port;
    uint16_t     sda_pin;
} SoftI2C_Bus;

/* 48MHz参考循环次数；初始化时按实际主频缩放，不代表精确总线频率。 */
#ifndef SOFT_I2C_DELAY_LOOPS
#define SOFT_I2C_DELAY_LOOPS   30u
#endif

/* 开启端口时钟，配置开漏输出和内部上拉，并释放两条线；仍需合适的外部上拉。 */
void SoftI2C_Init(const SoftI2C_Bus *bus);

/* 使用八位写地址发送数据，地址或数据未应答时停止并返回一。 */
uint8_t SoftI2C_Write(const SoftI2C_Bus *bus, uint8_t dev_addr,
                      const uint8_t *data, uint16_t len);

/* 发送八位写地址和寄存器索引，再重复起始切换为读方向；返回零表示传输成功。 */
uint8_t SoftI2C_WriteRead(const SoftI2C_Bus *bus, uint8_t dev_addr,
                          const uint8_t *wr, uint16_t wr_len,
                          uint8_t *rd, uint16_t rd_len);

#ifdef __cplusplus
}
#endif

#endif
