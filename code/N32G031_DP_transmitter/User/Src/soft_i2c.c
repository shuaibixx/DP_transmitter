/* 软件两线总线驱动：开漏引脚产生起止、数据与应答时序，两路总线共用实现。 */
#include "soft_i2c.h"

/* 延时循环在总线初始化时按实际主频缩放，以48MHz时30次循环为参考。 */
static uint32_t s_delay_loops = 1u;

static void i2c_delay(void)
{
    volatile uint32_t n = s_delay_loops;
    while (n-- != 0u)
    {
    }
}
/* 释放开漏时钟线，由外部或内部上拉拉高。 */
static void scl_release(const SoftI2C_Bus *bus)
{
    bus->scl_port->PBSC = bus->scl_pin;
}

/* 主动将时钟线拉低。 */
static void scl_low(const SoftI2C_Bus *bus)
{
    bus->scl_port->PBC = bus->scl_pin;
}

/* 释放开漏数据线，供从机驱动或上拉。 */
static void sda_release(const SoftI2C_Bus *bus)
{
    bus->sda_port->PBSC = bus->sda_pin;
}

/* 主动将数据线拉低。 */
static void sda_low(const SoftI2C_Bus *bus)
{
    bus->sda_port->PBC = bus->sda_pin;
}

/* 读取数据线实际电平并归一为零或一。 */
static uint8_t sda_level(const SoftI2C_Bus *bus)
{
    return ((bus->sda_port->PID & bus->sda_pin) != 0u) ? 1u : 0u;
}

/* 释放时钟并有限等待线路升高；当前超时后仍继续，未向上层返回错误。 */
static void scl_release_and_wait(const SoftI2C_Bus *bus)
{
    uint32_t guard = 2000u;

    scl_release(bus);
    while (((bus->scl_port->PID & bus->scl_pin) == 0u) && (guard != 0u))
    {
        guard--;
    }
    i2c_delay();
}

/* 根据端口选择对应外设时钟，仅支持本芯片实际存在的端口。 */
static uint32_t port_clock_bit(GPIO_Module *port)
{
    if (port == GPIOA) { return RCC_APB2_PERIPH_GPIOA; }
    if (port == GPIOB) { return RCC_APB2_PERIPH_GPIOB; }
    if (port == GPIOC) { return RCC_APB2_PERIPH_GPIOC; }
    if (port == GPIOF) { return RCC_APB2_PERIPH_GPIOF; }
    return 0u;
}

/* 时钟为高时拉低数据线，生成起始条件。 */
static void i2c_start(const SoftI2C_Bus *bus)
{
    sda_release(bus);
    scl_release_and_wait(bus);
    i2c_delay();

    sda_low(bus);
    i2c_delay();
    scl_low(bus);
    i2c_delay();
}

/* 时钟为高时释放数据线，生成停止条件。 */
static void i2c_stop(const SoftI2C_Bus *bus)
{
    sda_low(bus);
    i2c_delay();

    scl_release_and_wait(bus);
    sda_release(bus);
    i2c_delay();
}

/* 高位先发送八位数据，第九个时钟读取应答；零为应答，一为未应答。 */
static uint8_t i2c_write_byte(const SoftI2C_Bus *bus, uint8_t value)
{
    uint8_t i;
    uint8_t ack;

    for (i = 0u; i < 8u; i++)
    {
        if ((value & 0x80u) != 0u)
        {
            sda_release(bus);
        }
        else
        {
            sda_low(bus);
        }
        i2c_delay();
        scl_release_and_wait(bus);
        scl_low(bus);
        i2c_delay();

        value = (uint8_t)(value << 1);
    }

    sda_release(bus);
    i2c_delay();
    scl_release_and_wait(bus);
    ack = sda_level(bus);
    scl_low(bus);
    i2c_delay();

    return ack;
}

/* 高位先接收一个字节；参数非零时回送应答，最后一字节应回送非应答。 */
static uint8_t i2c_read_byte(const SoftI2C_Bus *bus, uint8_t ack)
{
    uint8_t i;
    uint8_t value = 0u;

    sda_release(bus);

    for (i = 0u; i < 8u; i++)
    {
        value = (uint8_t)(value << 1);

        scl_release_and_wait(bus);
        if (sda_level(bus) != 0u)
        {
            value |= 1u;
        }
        scl_low(bus);
        i2c_delay();
    }

    if (ack != 0u)
    {
        sda_low(bus);
    }
    else
    {
        sda_release(bus);
    }
    i2c_delay();
    scl_release_and_wait(bus);
    scl_low(bus);
    i2c_delay();
    sda_release(bus);

    return value;
}

/* 开启端口时钟，配置开漏输出和内部上拉，并释放两条线；仍需合适的外部上拉。 */
void SoftI2C_Init(const SoftI2C_Bus *bus)
{
    GPIO_InitType init;
    uint32_t       clocks;

    /* 只在初始化时做比例计算，避免逐位传输中执行除法；8MHz时为5次。 */
    s_delay_loops = (SOFT_I2C_DELAY_LOOPS * (SystemCoreClock / 1000000u) + 47u) / 48u;
    if (s_delay_loops == 0u) s_delay_loops = 1u;

    clocks = port_clock_bit(bus->scl_port) | port_clock_bit(bus->sda_port);
    RCC_EnableAPB2PeriphClk(clocks | RCC_APB2_PERIPH_AFIO, ENABLE);

    GPIO_InitStruct(&init);
    init.GPIO_Mode      = GPIO_MODE_OUTPUT_OD;
    init.GPIO_Pull      = GPIO_PULL_UP;
    init.GPIO_Speed     = GPIO_SPEED_HIGH;
    init.GPIO_Current   = GPIO_DC_LOW;
    init.GPIO_Alternate = 0u;

    init.Pin = bus->scl_pin;
    GPIO_InitPeripheral(bus->scl_port, &init);

    init.Pin = bus->sda_pin;
    GPIO_InitPeripheral(bus->sda_port, &init);

    scl_release(bus);
    sda_release(bus);
}

/* 使用八位写地址发送数据，地址或数据未应答时停止并返回一。 */
uint8_t SoftI2C_Write(const SoftI2C_Bus *bus, uint8_t dev_addr,
                      const uint8_t *data, uint16_t len)
{
    uint16_t i;

    i2c_start(bus);

    if (i2c_write_byte(bus, dev_addr) != 0u)
    {
        i2c_stop(bus);
        return 1u;
    }

    for (i = 0u; i < len; i++)
    {
        if (i2c_write_byte(bus, data[i]) != 0u)
        {
            i2c_stop(bus);
            return 1u;
        }
    }

    i2c_stop(bus);
    return 0u;
}

/* 发送八位写地址和寄存器索引，再重复起始切换为读方向；返回零表示传输成功。 */
uint8_t SoftI2C_WriteRead(const SoftI2C_Bus *bus, uint8_t dev_addr,
                          const uint8_t *wr, uint16_t wr_len,
                          uint8_t *rd, uint16_t rd_len)
{
    uint16_t i;

    if ((rd == 0) || (rd_len == 0u))
    {
        return 1u;
    }

    i2c_start(bus);

    if (i2c_write_byte(bus, dev_addr) != 0u)
    {
        i2c_stop(bus);
        return 1u;
    }

    for (i = 0u; i < wr_len; i++)
    {
        if (i2c_write_byte(bus, wr[i]) != 0u)
        {
            i2c_stop(bus);
            return 1u;
        }
    }

    sda_release(bus);
    scl_release_and_wait(bus);
    i2c_delay();
    sda_low(bus);
    i2c_delay();
    scl_low(bus);
    i2c_delay();

    if (i2c_write_byte(bus, (uint8_t)(dev_addr | 0x01u)) != 0u)
    {
        i2c_stop(bus);
        return 1u;
    }

    for (i = 0u; i < rd_len; i++)
    {
        rd[i] = i2c_read_byte(bus, (uint16_t)(i + 1u) < rd_len ? 1u : 0u);
    }

    i2c_stop(bus);
    return 0u;
}
