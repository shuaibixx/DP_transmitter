/* 调试串口配置：发送脚为PA2，接收脚为PA3，默认波特率为115200。 */
#ifndef __USART_H__
#define __USART_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "n32g031.h"

#define USART_DEBUG_PORT        GPIOA
#define USART_DEBUG_TX_PIN      GPIO_PIN_2
#define USART_DEBUG_RX_PIN      GPIO_PIN_3
#define USART_DEBUG_AF          GPIO_AF4_USART2

#define USART_DEBUG_BAUDRATE    115200u

/* 配置第二串口及引脚复用，采用八位数据、无校验、一位停止位。 */
void USART_Debug_Init(void);

/* 发送以空字符结束的字符串；发送时间计入主循环周期。 */
void USART_SendString(const char *str);

/* 等待发送寄存器可用，再阻塞等待本字节发送结束。 */
void USART_SendByte(uint8_t byte);

#ifdef __cplusplus
}
#endif

#endif
