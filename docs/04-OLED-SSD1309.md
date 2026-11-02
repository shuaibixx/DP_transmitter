# 04 — OLED SSD1309 128×64

> 2026-10-27最新：用户要求恢复显示和GP315，当前先重新启用本SSD1309 OLED驱动，与已修正8MHz软件I²C的传感器联动。新断码屏方案仍待型号/接口；本次整机版本待上板验证。

> 2026-10-27：用户计划更换断码屏，旧OLED暂停排查，本文作为历史驱动资料保留。当前只测试传感器，不扫描或初始化OLED。新屏型号和接口待提供，尚未编写新驱动。

> 2026-10-22状态补充：此前花屏/无显示后，用户已确认显示恢复正常；0x3C已应答，采集显示模式已接入实时压力和温度，使用清主控显存、重绘后刷新以避免残留。当前GP315独立测试不初始化OLED。下文早期界面和未验证记录仅供历史对照。

1.54 寸 4 针 OLED 模块，**SSD1309** 控制器，分辨率 **128×64**。
驱动源码来自商家提供的 STM32/IAP15 例程（未加密，可读），已适配到本工程。

---

## 1. 硬件

| 项 | 值 |
|---|---|
| 控制器 | **SSD1309** |
| 分辨率 | **128 × 64**（初始化用 1/64 duty，`0xA8 0x3F`） |
| 接口 | I²C |
| 模块引脚 | VCC / GND / SCL / SDA —— **只有 4 针，没有 RES 脚** |
| I²C 地址字节 | **0x78**（7bit = **0x3C**） |
| 控制字节 | 命令 `0x00`，数据 `0x40` |
| 本工程接线 | **PA6 = SCL，PA7 = SDA**（软件模拟 I²C） |

> **没有 RES 脚** → 不能做硬复位，依赖 SSD1309 的内部上电复位。
> 所以 `OLED_Init()` 里只做 `Delay_ms(50)` 等上电稳定，不做复位时序。

---

## 2. 初始化序列（`oled.c` 的 `OLED_Init`）

从商家例程逐条搬运，已核对：

```
0xAE              display off
0xFD 0x12         command lock: 允许后续命令
0xD5 0xA0         display clock divide / oscillator
0xA8 0x3F         multiplex ratio = 1/64
0xD3 0x00         display offset = 0
0x40              display start line = 0
0xA1              segment remap（左右方向）
0xC8              COM scan direction remapped（上下方向）
0xDA 0x12         COM pins hardware config
0x81 0xBF         对比度
0xD9 0x25         pre-charge period
0xDB 0x34         VCOMH deselect level
0xA4              从 RAM 内容恢复显示
0xA6              正常显示（非反色）
OLED_Clear()
0xAF              display on
```

---

## 3. 显存组织

- **页寻址模式**：`s_gram[128][8]`，每个字节 = 一列里的 8 个像素（一页）
- **bit0 = 该页最上面一行**
- 刷新时按页写：先发页地址 `0xB0+page`、列地址低/高半字节，再连续写 128 字节数据

```c
OLED_Refresh():
for (page = 0..7) {
    命令 0xB0 + page        /* 页地址 */
    命令 0x00               /* 列地址低 4 位 */
    命令 0x10               /* 列地址高 4 位 */
    数据 0x40 + s_gram[0..127][page]
}
```

- 所有绘图函数**只改显存**，`OLED_Refresh()` 才推到屏幕
- `OLED_Clear()` 也只清显存，要配合 `OLED_Refresh()`

---

## 4. 字库

`oledfont.h` 是从商家例程里**只提取实际用到的两个字库**生成的（原文件 44 KB）：

| 数组 | 尺寸 | 用途 |
|---|---|---|
| `asc2_0806` | 6×8 ASCII | 小字（标签、单位） |
| `asc2_1608` | 8×16 ASCII | 大字（数值） |

由 `D:\tmp\extract_font.js` 提取（脚本思路：从原 `oledfont.h` 里按名字抓到 `};` 为止）。

### ⚠️ 商家例程的字库越界 bug

原例程的 `asc2_0806` **只有 92 项**（覆盖 `' '` 到 `'z'` + 一条横线图案），
但例程 `OLED_ShowChar()` 里是 `index = chr - ' '`，显示 `'~'`（0x7E）时 index = 94 → **越界读**。

**本工程的修复**：`oled.c` 里

```c
#define FONT_6X8_CHARS   (sizeof(asc2_0806) / sizeof(asc2_0806[0]))
#define FONT_8X16_CHARS  (sizeof(asc2_1608) / sizeof(asc2_1608[0]))
...
if (index >= FONT_6X8_CHARS) index = 0u;   /* 退回空格 */
```

用 `sizeof` 让编译器算实际长度，不写死数字。

---

## 5. API

```c
void OLED_Init(void);
void OLED_Clear(void);                       /* 只清显存 */
void OLED_Refresh(void);                     /* 推到屏幕 */
void OLED_DrawPoint(uint8_t x, uint8_t y, uint8_t on);
void OLED_ShowChar(uint8_t x, uint8_t y, char ch, uint8_t size, uint8_t mode);
void OLED_ShowString(uint8_t x, uint8_t y, const char *str, uint8_t size, uint8_t mode);
void OLED_ShowNum(uint8_t x, uint8_t y, int32_t num, uint8_t len, uint8_t size);
void OLED_ShowFloat(uint8_t x, uint8_t y, float value, uint8_t frac_digits, uint8_t size);
```

- `size`: `OLED_FONT_6X8`（8）或 `OLED_FONT_8X16`（16）
- `mode`: `1` = 正常，`0` = 反色
- 字符步进：6×8 每字符 6 像素宽，8×16 每字符 8 像素宽

---

## 6. 当前界面布局（`main.c`）

```
y=0    DP Transmitter        ← 6x8
y=16   P: 12.3 kPa           ← 8x16
y=32   T: 25.6 C
y=48   I: 10.5 mA
```

- 标签固定，只刷新数值区，最后统一 `OLED_Refresh()`
- 数值用 `OLED_ShowFloat(x, y, value, 1, OLED_FONT_8X16)`

---

## 7. 验证状态

**已上板验证恢复正常；长期稳定性与整机联调待验证。**

总线扫描时 `display PA6/PA7 ... ack: (none)` —— 当时 OLED 没接（用户明确说先不管 OLED）。
接上后扫描应该出现 **`0x3C`**。

### 上板检查顺序

1. 扫描里出现 `0x3C` → I²C 通了
2. 屏幕全黑 → 检查 `0xAF`（display on）和对比度
3. 花屏/错位 → 检查 `0xA1` / `0xC8` 的扫描方向
4. 只显示一半 → 检查 `0xA8 0x3F`（multiplex）和 `0xDA 0x12`

---

## 8. 资料位置

商家例程原文（未加密，可直接读）：

```
datum/1.54寸4针白色OLED屏/02-1.54OLED程序源码/STM32例程/
    01-1.54OLED显示屏STM32F103C8T6_IIC例程/HARDWARE/OLED/
        oled.c  oled.h  oledfont.h  bmp.h
```

规格书（**加密，需提取**）：`datum/1.54寸4针白色OLED屏/01-规格书与控制芯片手册/SSD1309.pdf`
