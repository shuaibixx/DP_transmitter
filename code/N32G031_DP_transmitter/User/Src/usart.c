/* 调试串口驱动：使用第二串口，阻塞发送字节和字符串。 */
#include "usart.h"

/* 等待发送寄存器可用，再阻塞等待本字节发送结束。 */
void USART_SendByte(uint8_t byte)
{
    while (USART_GetFlagStatus(USART2, USART_FLAG_TXDE) == RESET)
    {
    }

    USART_SendData(USART2, (uint16_t)byte);

    while (USART_GetFlagStatus(USART2, USART_FLAG_TXC) == RESET)
    {
    }
}

/* 发送以空字符结束的字符串；发送时间计入主循环周期。 */
void USART_SendString(const char *str)
{
    if (str == 0)
    {
        return;
    }

    while (*str != '\0')
    {
        USART_SendByte((uint8_t)(*str));
        str++;
    }
}

/* 配置第二串口及引脚复用，采用八位数据、无校验、一位停止位。 */
void USART_Debug_Init(void)
{
    GPIO_InitType  gpio;
    USART_InitType usart;

    RCC_EnableAPB1PeriphClk(RCC_APB1_PERIPH_USART2, ENABLE);
    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOA | RCC_APB2_PERIPH_AFIO, ENABLE);

    GPIO_InitStruct(&gpio);
    gpio.Pin            = USART_DEBUG_TX_PIN;
    gpio.GPIO_Mode      = GPIO_MODE_AF_PP;
    gpio.GPIO_Pull      = GPIO_NO_PULL;
    gpio.GPIO_Speed     = GPIO_SPEED_HIGH;
    gpio.GPIO_Current   = GPIO_DC_LOW;
    gpio.GPIO_Alternate = USART_DEBUG_AF;
    GPIO_InitPeripheral(USART_DEBUG_PORT, &gpio);

    gpio.Pin            = USART_DEBUG_RX_PIN;
    gpio.GPIO_Mode      = GPIO_MODE_INPUT;
    gpio.GPIO_Pull      = GPIO_PULL_UP;
    gpio.GPIO_Alternate = 0u;
    GPIO_InitPeripheral(USART_DEBUG_PORT, &gpio);

    USART_StructInit(&usart);
    usart.BaudRate            = USART_DEBUG_BAUDRATE;
    usart.WordLength          = USART_WL_8B;
    usart.StopBits            = USART_STPB_1;
    usart.Parity              = USART_PE_NO;
    usart.Mode                = USART_MODE_RX | USART_MODE_TX;
    usart.HardwareFlowControl = USART_HFCTRL_NONE;
    USART_Init(USART2, &usart);

    USART_Enable(USART2, ENABLE);
}
