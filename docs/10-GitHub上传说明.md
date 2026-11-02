# GitHub上传说明

整理日期：2026-11-02。目标仓库：https://github.com/shuaibixx/DP_transmitter 。用户要求上传内容不加密。

## 收录与验证

源码、工程所需CMSIS/外设库、项目文档、调试记录、可读取的厂商示例及支持格式资料纳入上传。当前工程的15个编译源文件和4个头文件搜索目录均位于工程内部，不依赖datum目录；安装Keil ARMCC5和设备包后可编译。

扫描已识别的加密容器头，并对文本执行可解码性检查；PDF、图片、ZIP等支持格式执行文件头或容器检查。初次归档扫描通过1854个候选文件，约33898514字节（不含后生成的说明与清单）。Git提交前后再检查Git实际保存的文件内容，避免把本机可读误当成仓库内容可读。

优先使用的[新版传感器手册](datasheets/HBP5023D-2压力传感器模组_新版.md)已从用户指定的原文来源归档为明文，内容保留原样，未据此改动驱动。用户先前提供的原理图片段已归档4张，见[GP315电路](hardware-reference/gp315_circuit.png)、[电源](hardware-reference/power_circuit.png)、[MCU引脚](hardware-reference/mcu_pinout.png)、[连接器](hardware-reference/connectors_circuit.png)。截图只提供片段参考，不能替代完整SchDoc/PcbDoc工程。

## 未上传原件

初次扫描发现25个加密或未通过格式验证的原件，已逐文件加入.gitignore；文件仍保留本地，没有删除、改写或更名。部分原始PDF、SchDoc/PcbDoc带相同的加密容器头，不能按扩展名认定可读，也没有把密文改后缀伪装成明文。其可用文字提取稿保留在docs/datasheets，但不保证覆盖全部图表。旧版手册提取稿中的5个NUL控制字节已移除以便作为文本归档；原始提取稿备份保留在仓库外的D:/code/DP_transmitter_local-originals_20261102/。

datum根目录未上传项：

| 本地相对路径 | 未上传原因 |
|---|---|
| `datum/CN_DS_N32G031_Series_Datasheet_V1.5.1.pdf` | encrypted container header |
| `datum/GP315-CN-V1.23.pdf` | encrypted container header |
| `datum/GP315四段校正_PWM输入0-25mA输出.pdf` | encrypted container header |
| `datum/HBDP-4001.PcbDoc` | encrypted container header |
| `datum/HBDP-4001.SchDoc` | encrypted container header |
| `datum/HBP-4001.pdf` | encrypted container header |
| `datum/HBP5023D-1.pdf` | encrypted container header |
| `datum/HBP5023D-2.pdf` | encrypted container header |

完整逐文件清单见[publication-exclusions.json](publication-exclusions.json)，其中同时记录构建产物和本机IDE缓存。清单记录的是首次准备时的本地文件，不是远端文件列表。后续若能通过原软件正常导出可读原件，再验证、移除相应忽略规则并提交。

## 后续提交

1. 更新后先暂存需要提交的文件。
2. 运行`python tools/check_plaintext.py`验证暂存区实际Git内容；有BLOCKED就停止提交上传。
3. 提交后运行`python tools/check_plaintext.py --revision HEAD`复查提交。

该检查覆盖已知加密容器头与受支持格式，不是对任意未知加密形式的数学证明。未验证格式默认不上传。本次没有修改固件逻辑、量程或标定参数。

## 首次上传验证结果

- 暂存区及提交对象共1854个文件，通过Git实际blob内容检查；已知加密头为0，未通过格式验证项为0。
- 在独立目录从本地提交重新clone，并用Keil ARMCC5全量编译，0错误、0警告；Code=15908、RO=320、RW=28、ZI=1028。编译未改变固件逻辑或标定参数。
- 本机DLP会对Git检出后新写入的启动汇编.s文件再次加密：首次编译因此失败。已核对提交内该文件是明文，将同一Git blob按原始字节写回验证目录后编译通过。此问题属于本机检出后的保护行为，不是仓库内保存密文。受同类环境管理的电脑下载后应另行确认本地文件状态；不要把本机受保护的字节重新提交。
- 14个已识别加密原件和11个格式未验证文件没有纳入提交，具体路径见清单。构建产物和IDE缓存同样忽略。
