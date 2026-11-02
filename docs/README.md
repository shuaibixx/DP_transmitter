# DP_transmitter — 差压变送器

N32G031 + HBP5023D-2数字差压传感器 + GP315 PWM转电流输出。当前配置0～20kPa差压采集及4～20mA输出；main.c中的OLED调用已删除。

更新日期：2026-10-30。本文只维护项目入口、编译方法和文档索引；当前配置、实测结果、临时试测设置及待办统一以[当前项目进度](00-当前项目进度.md)为准。目标功能不代表已经完成整机验收。

最新进展：已收到0／5／10／15／20kPa两轮联动实测，最新五点最大整机电流偏差0.04mA（16mA输出跨度的0.25%），仍需独立复核4mA零点及重复性；详见[实测记录](09-压力与电流实测记录.md)。这不是整机精度验收结论。

## 先读什么

1. [当前项目进度](00-当前项目进度.md)：已验证内容、当前固件模式、最新实测和下一步。
2. 按需阅读下方模块文档。
3. [调试记录与待办](06-调试记录与待办.md)：历史经过及当前待办；早期记录不覆盖最新结论。

## 工程位置

路径相对于项目根目录 `D:/code/DP_transmitter`，本文位于 `docs/README.md`。

| 路径 | 内容 |
|---|---|
| `code/N32G031_DP_transmitter/N32G031_DP_transmitter.uvprojx` | Keil工程入口 |
| `code/N32G031_DP_transmitter/User/Src` | 应用源文件 |
| `code/N32G031_DP_transmitter/User/Inc` | 应用头文件及配置 |
| `code/N32G031_DP_transmitter/Drivers` | 芯片及标准外设库 |
| `code/N32G031_DP_transmitter/Objects/N32G031_DP_transmitter.hex` | 编译后的烧录文件 |
| `docs` | 项目说明、进度和调试记录 |
| `docs/datasheets` | 已提取的手册文本，用前核对来源优先级 |
| `datum` | 原始资料、原理图、PCB和厂家示例 |

## 编译与调试入口

当前工程使用Keil MDK、ARMCC V5.06 update6（build750），设备包为 `Nationstech.N32G031_DFP.1.0.6`，目标器件N32G031K8。

可直接在Keil中打开工程并编译。当前本机PowerShell命令示例：

```powershell
$buildProcess = Start-Process -FilePath 'D:/Keil/UV4/UV4.exe' `
    -ArgumentList '-b N32G031_DP_transmitter.uvprojx -j0 -o build.log' `
    -WorkingDirectory 'D:/code/DP_transmitter/code/N32G031_DP_transmitter' `
    -WindowStyle Hidden -PassThru -Wait
Get-Content 'D:/code/DP_transmitter/code/N32G031_DP_transmitter/build.log'
```

检查日志的错误和警告；不要仅凭旧HEX文件存在判断本次编译成功。迁移电脑后需按实际安装位置调整命令。

串口：USART2，PA2=TX、PA3=RX，115200、8N1。`User/Inc/main.h`中的APP_GP315_CAL_CURRENT_MA设为0.0f时按压力输出电流，设为正值时固定输出该mA值；两点实测电流宏任意一个为0则跳过自动标定。切换前请核对[进度总览](00-当前项目进度.md)。

## 文档索引

| 文档 | 内容 |
|---|---|
| [00 当前项目进度](00-当前项目进度.md) | 当前状态、配置、实测、限制及下一步；状态唯一入口 |
| [01 硬件与引脚](01-硬件与引脚.md) | MCU、时钟、接线、电平和供电 |
| [02 传感器HBP5023D-2](02-传感器-HBP5023D-2.md) | I²C、寄存器、SCO及压力温度换算 |
| [03 GP315 PWM输出](03-GP315-PWM输出.md) | 电流换算、厂家补偿、PWM诊断及标定边界 |
| [04 OLED SSD1309](04-OLED-SSD1309.md) | 显存、字库、显示接口及验证情况 |
| [05 软件架构](05-软件架构.md) | 模块、API、流程及配置入口 |
| [06 调试记录与待办](06-调试记录与待办.md) | 历史排查过程与后续工作 |
| [07 资料与环境](07-资料与环境.md) | 手册来源优先级、读取限制及开发环境 |
| [08 GP315宏标定](08-GP315宏标定.md) | 在main.h填写两点实测电流、零值跳过和标定步骤 |
| [09 压力与电流实测记录](09-压力与电流实测记录.md) | 两轮五点实测原始数据、误差计算和使用边界 |
| [10 GitHub上传说明](10-GitHub上传说明.md) | 明文验证、归档范围和未上传原件清单 |

## 资料使用约定

传感器优先手册路径见[资料与环境](07-资料与环境.md)，旧提取文件只作对照；寄存器位图结合用户补充表4和项目验证判断。原始资料可能受本机DLP环境影响，复制或打包不保证在其他电脑可读，不再按文件扩展名一概认定明文可读。
