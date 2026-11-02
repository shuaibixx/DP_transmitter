# 原始手册（已提取为明文）

> 2026-11-02归档说明：优先使用本目录的[HBP5023D-2压力传感器模组_新版.md](HBP5023D-2压力传感器模组_新版.md)。旧提取文本保留作历史对照。当前本地datum确有D-2原始PDF，但带加密容器头，未上传；本文“全文”等历史用语不保证图片表格完整，实际以提取内容及项目验证为准。

公司 DLP 会把 `.pdf` 加密，**直接读 `datum/` 里的 PDF 只会得到乱码**。
这里是从 TRAE 提取结果里复制过来的**明文全文**，可以直接 Read / grep。

| 文件 | 来源 | 说明 |
|---|---|---|
| `HBP5023D-2_数据手册.md` | TRAE 提取 | **传感器手册全文**。⚠️ 表4「内部寄存器信息」提取时丢失（原 PDF 里应是图片） |
| `GP315_数据手册_V1.23.md` | TRAE 提取 | **GP315 芯片手册全文**：PWM 50Hz~50kHz、0.5~25mA、VCC 8~30V |
| `GP315_四段校正应用说明.md` | TRAE 提取 | 四段补偿算法 + **完整可复制的 C 函数** |
| `N32G031_datasheet_text/pages/page_00NN.txt` | TRAE 提取 | N32G031 手册逐页文字（共 78 页） |
| `N32G031_datasheet_text/pin_table.txt` | TRAE 提取 | 引脚表（表格形式，但**列会错位**） |
| `N32G031_datasheet_text/pin_words_p32.txt` ~ `p36.txt` | TRAE 提取 | **带 x/y 坐标的引脚表文字**，判断列归属最可靠 |

## 怎么用 `pin_words_p*.txt`

每行格式：

```
top=  269.2 x0=  247.7 x1=  263.7  PA4
top=  269.2 x0=  382.8 x1=  427.6  TIM1_CH1,
top=  269.2 x0=  471.2 x1=  521.1  COMP_INM,
```

- `top` = 垂直位置 → **同一行的内容 `top` 相同**
- `x0` = 水平位置 → **区分列**

N32G031 引脚表的列位置（大致）：

| x0 | 列 |
|---|---|
| 63 ~ 210 | 各封装的引脚号（LQFP48 / LQFP32 / QFN32×2 / UFQFPN20 / TSSOP20） |
| **247** | **引脚名**（如 PA4） |
| 301 | 类型 I/O |
| 334 | I/O 结构 TC |
| **382** | **复用功能**（Alternate functions） |
| **471** | **附加功能**（Additional functions） |

用这个就能准确还原「某个引脚支持哪些复用功能」，不会像 `pin_table.txt` 那样串列。

## 原始位置（TRAE）

```
C:\Users\Lenovo\AppData\Roaming\TRAE SOLO CN\ModularData\ai-agent\work-mode-projects\
├── 6a8bf7b3c916dd1b8a239724\    ← N32G031 + 旧批次
└── 6ad0a3a7aadd7406530abaa0\    ← HBP5023D-2 + GP315 手册 + STC8H
```

那里可能还有本项目以后需要的资料，优先去翻。

## 缺失的资料

- **HBP5023D-2 原 PDF** —— `datum/` 里只有旧款 `HBP5023D-1.pdf`
- **表4「内部寄存器信息」** —— 提取文本里是空的，需要原 PDF 用 WPS 打开看
