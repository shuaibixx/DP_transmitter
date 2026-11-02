# DP_transmitter

基于 N32G031 的数字差压变送器，使用 HBP5023D-2 采集压力和温度，通过 GP315 输出对应的 4～20mA 电流，支持量程配置、串口调试和两点电流标定。

## 当前配置

- MCU：内部 HSI 8MHz，PLL 关闭。
- 传感器：HBP5023D-2，当前量程 0～20kPa；软件 I²C，PF1=SCL、PF0=SDA，地址 0x6D。
- 输出：GP315；PA4 / TIM3_CH1，PWM 1kHz，每周期 8000 计数。
- 串口：USART2，PA2=TX、PA3=RX，115200 8N1，输出 AD、压力、温度和目标电流。
- main.c 已移除屏幕调用；旧 OLED 驱动保留，后续断码屏待选型。

当前换算：`Iset(mA) = 4 + P(kPa) × 0.8`，输出目标限幅 4～20mA。Iset 是软件目标值，实际电流需外部万用表测量。

## 编译与配置

1. 安装 Keil MDK，使用 ARMCC V5.06 update 6（build 750）及设备包 `Nationstech.N32G031_DFP.1.0.6`。
2. 打开 `code/N32G031_DP_transmitter/N32G031_DP_transmitter.uvprojx` 并编译。
3. 烧录生成的 `Objects/N32G031_DP_transmitter.hex`，烧录器及接线按实际硬件配置。

常改配置在 [main.h](code/N32G031_DP_transmitter/User/Inc/main.h)：量程、GP315 两点基准实测电流和固定标定目标。`APP_GP315_CAL_CURRENT_MA=0` 启用压力联动，正值固定输出该目标 mA。更换板子后按[宏标定步骤](docs/08-GP315宏标定.md)重新测量，不能直接沿用另一块板的标定数据。

## 目录与文档

| 目录 | 内容 |
|---|---|
| `code/N32G031_DP_transmitter` | Keil 工程、应用源码、CMSIS 与所需外设库 |
| `docs` | 项目进度、硬件、算法、调试及实测记录 |
| `docs/datasheets` | 手册明文提取资料及优先使用的新版传感器手册 |
| `docs/hardware-reference` | 用户提供的原理图片段截图 |
| `datum` | 经检查可读的原始资料及厂商示例；加密原件未上传 |
| `tools/check_plaintext.py` | 检查暂存区或指定提交中的实际 Git 文件内容 |

先读[当前项目进度](docs/00-当前项目进度.md)，完整索引见[docs/README](docs/README.md)。两轮五点实测见[实测记录](docs/09-压力与电流实测记录.md)。现有测试不等同于整机精度验收。

## 明文上传约定

本仓库只收录可验证的明文源码、文档及可读格式资料。编译产物、IDE 缓存、加密原件及暂不能验证的文件不上传；详情及逐文件清单见[上传说明](docs/10-GitHub上传说明.md)。本地原件保留，未修改固件逻辑或标定参数。

推送前可运行 `python tools/check_plaintext.py` 检查暂存区；提交后运行 `python tools/check_plaintext.py --revision HEAD` 检查提交。检查针对已知加密文件头、文本可解码性及受支持文件格式；新格式需要补充验证。

厂商源码的原有版权及许可声明保留在各文件中；本仓库未额外授予第三方资料的许可。
