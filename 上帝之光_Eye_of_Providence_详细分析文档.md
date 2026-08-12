
# "上帝之光"（Eye of Providence）摄像头代码详细分析文档

---

## 一、命名由来

### 1.1 源代码中的名称

在项目代码中，这套摄像头图像处理算法的源文件名为：
- `code/Eye_of_Providence.c` — 算法实现（1397行）
- `code/Eye_of_Providence.h` — 接口定义与数据结构（182行）

**"Eye of Providence"** 直译为"全视之眼"或"上帝之眼"（Providence 在基督教神学中指"上帝的眷顾/天意"）。这是共济会标志中著名的"上帝全视之眼"符号——一只眼睛位于三角形顶端，象征上帝洞悉一切、无所不见。

### 1.2 为什么叫"上帝之光"

团队成员将这套算法称为**"上帝之光"**，寓意是：就像上帝的光芒能够照亮一切黑暗、看穿一切迷雾一样，这套视觉算法能够在各种极端光照条件下（强光直射、地面反光、光照不均匀、阴影遮挡）准确地识别赛道，帮助小车"看清"前路。这个名字带有一定的夸张和自信色彩，但从代码设计来看，它确实在光照鲁棒性方面做了非常深入的工作。

---

## 二、完整图像处理流水线

主函数 `Eye_of_Providence_Task()` 定义了图像处理的完整流程，每一步按顺序执行：

```
Eye_of_Providence_Task() 执行流程：
┌──────────────────────────────────────────────────────────────┐
│  1. copy_img()            复制图像，裁去左右边缘             │
│  2. img_compress()        压缩图像 160×120 → 80×60          │
│  3. img_compress2()       压缩显示用原图                     │
│  4. gamma_correction()    对压缩后图像进行 Gamma 校正        │
│  5. image_data_init()     图像数据初始化，清零标志位          │
│  6. binary_by_part()      分区域大津法二值化                  │
│  7. DrawLinesFirst()      绘制底边（第59行），建立基准       │
│  8. DrawLinesProcess()    自底向上逐行追踪赛道边沿            │
│  9. Fine_whitepoint()     计算最长白色连续行（确定有效区域）  │
│ 10. Get_TowPoint()        计算动态前瞻点                      │
│ 11. Find_Spinodal()       寻找拐点（用于元素识别）            │
│ 12. Element_Judgment()    赛道元素类型判断                    │
│ 13. RouteFilter()         路径滤波（修正异常中心线）          │
│ 14. Element_Handle()      元素处理（执行转弯/直行动作）       │
│ 15. Get_Deviation()       计算加权偏差（用于转向控制）        │
└──────────────────────────────────────────────────────────────┘
```

下面逐步骤详细分析。

---

## 三、逐步骤技术详解

### 3.1 步骤1-2：图像采集与压缩（copy_img + img_compress）

```c
void copy_img(void) {
    for (int i = 0; i < 120; i++) {
        memcpy(&mt9_copy_image[i][0], &mt9v03x_image_1[i][8], 152);
    }
    mt9v03x_finish_flag_1 = 0;  // 允许下一帧DMA开始采集
}
```

**技术要点：**
- 从 160 列中裁去左边 8 列，得到 152 列的有效数据
- 清除 DMA 完成标志，触发下一帧采集（流水线操作）
- 使用 `memcpy` 逐行复制，充分利用 TC264 的硬件加速

`img_compress()` 通过行列比例映射实现 2:1 降采样，将 120×152 压缩为 60×80。这种降采样本身也是一种低通滤波，有助于减少图像噪声。

### 3.2 步骤4：Gamma 校正 —— 第一道光线防线

```c
float gamma_value = 0.5;   // gamma < 1.0 = 增强对比度

void gamma_init(float gamma) {
    for (int i = 0; i < 256; i++) {
        lut[i] = (unsigned char)(pow(i / 255.0, 1.0 / gamma) * 255);
    }
}

void gamma_correction(unsigned char* img, int size) {
    for (int i = 0; i < size; i++) {
        img[i] = lut[img[i]];
    }
}
```

**关键参数：** `gamma_value = 0.5`

**数学原理：**
- Gamma 校正公式：`输出 = 255 × (输入/255)^(1/gamma)`
- 当 gamma = 0.5 时，指数 = 1/0.5 = 2.0
- 输出值快速增长：输入 128 → 输出 ≈ 64，输入 200 → 输出 ≈ 157

**对强光鲁棒性的作用：**

当赛道受到强光直射或大面积反光时，图像整体偏亮（像素值偏高，直方图右移）。gamma < 1.0 的效果是**压缩亮部、拉伸暗部**：

| 原始像素值 | gamma=1.0（不变） | gamma=0.5（校正后） | 效果 |
|-----------|-------------------|---------------------|------|
| 50（暗）  | 50 | ~12（更暗） | 暗部进一步拉开 |
| 128（中） | 128 | ~64（偏暗） | 中间调下移 |
| 200（亮） | 200 | ~157（略暗） | 亮部被压缩 |
| 250（极亮）| 250 | ~245（几乎不变）| 极亮区基本保持 |

**实际效果：** 在强光场景中，赛道（黑色）和背景（白色）之间的对比度被显著增强。被强光"冲淡"的赛道边缘重新变得清晰可辨。这对后续的二值化步骤至关重要——如果图像对比度不够，任何二值化算法都无法正确分离赛道和背景。

代码注释中明确写道：`gamma<1.0, 对比度增强; gamma>1.0, 对比度减弱`。

### 3.3 步骤5：图像数据初始化（image_data_init）

```c
void image_data_init(void) {
    ImageStatus.OFFLine_Last = ImageStatus.OFFLine;  // 保存上一帧的顶边界
    ImageStatus.OFFLine = 2;                          // 顶边界默认值（第2行）
    ImageStatus.First_W_Point_L = 0;   // 左侧第一个白色无边点
    ImageStatus.First_W_Point_R = 0;   // 右侧第一个白色无边点
    ImageStatus.First_H_Point_L = 0;   // 左侧第一个黑色无边点
    ImageStatus.First_H_Point_R = 0;   // 右侧第一个黑色无边点
    ImageFrameDetect.spinodal_L_Ysite = 0;  // 左拐点
    ImageFrameDetect.spinodal_R_Ysite = 0;  // 右拐点
    ImageFrameDetect.image_center_x = 0;    // 图像中心X
    continue_lost_count = 0;                // 连续丢线计数清零

    // 将第59行到OFFLine行之间的所有行标记为未找到边沿 'F'
    for (Ysite = 59; Ysite >= ImageStatus.OFFLine; Ysite--) {
        ImageDeal[Ysite].IsLeftFind = 'F';
        ImageDeal[Ysite].IsRightFind = 'F';
        ImageDeal[Ysite].LeftBorder = 0;
        ImageDeal[Ysite].RightBorder = 79;
    }
}
```

**三态边沿分类系统：**

| 标志 | 含义 | 触发条件 | 处理策略 |
|------|------|----------|----------|
| `'T'` | True Edge（真实边沿） | 扫描范围内找到黑白跳变 | 直接使用该边沿位置 |
| `'W'` | White/No Border（白色无边） | 没找到边沿，且行中间是白色 | 可能是大弯道或反光导致的白底 |
| `'H'` | Black/No Border（黑色无边） | 没找到边沿，且行中间是黑色 | 可能是丢线、坡道、或障碍物 |
| `'F'` | Not Found（未搜索） | 初始化默认值 | 表示该行尚未进行边沿搜索 |

这个三态分类系统是后续所有边沿追踪和错误恢复的基础。

### 3.4 步骤6：分区域大津法二值化 —— 最核心的光照鲁棒性技术

这是整套算法中**对抗光照不均匀最重要的一步**。

#### 3.4.1 大津法（OTSU）原理与缺陷

`GetOSTU()` 实现了标准的大津法（Otsu's Method）——遍历 0~255 共 256 个灰度级别，找到一个阈值 T 使得前景（大于T）和背景（小于T）之间的**类间方差**最大化：

```c
// 类间方差公式
Sigma = OmegaBack × OmegaFore × (MicroBack - MicroFore)²
```

其中：
- `OmegaBack`：背景像素占总像素的比例
- `OmegaFore`：前景像素占总像素的比例
- `MicroBack`：背景像素的平均灰度
- `MicroFore`：前景像素的平均灰度

代码注释中明确指出了 OTSU 的缺陷：
> "OSTU算法在处理光照不均匀的图像的时候效果明显不好，因为用的是全局的像素信息"

#### 3.4.2 分区策略 —— 解决光照不均匀的核心创新

`binary_by_part()` 函数的精妙之处在于：**不是用一个全局阈值，而是将 60 行图像按纵向分成 3 个区域，每个区域使用不同的二值化阈值：**

```c
void binary_by_part(void) {
    // 第一步：计算全局 OTSU 阈值
    threshold_total = GetOSTU((uint8_t *)mt9_compress_image, 80, 60);

    // 第二步：阈值限幅保护
    if(threshold_total < OSTU_Low)  threshold_total = OSTU_Low;   // 不低于 10
    else if(threshold_total > OSTU_High) threshold_total = OSTU_High; // 不高于 150

    // 第三步：为三个区域分配不同的阈值
    threshold_far    = threshold_total - 2;   // 远端：阈值更低
    threshold_middle = threshold_total - 1;   // 中间：过渡
    threshold_near   = threshold_total + 1;   // 近端：阈值更高

    // 第四步：分区域二值化
    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 80; j++) {
            // 远端（第0-19行）：使用 threshold_far
            if (mt9_compress_image[i][j] >= threshold_far)
                mt9_binary_image[i][j] = 1;    // 白色（赛道外）
            else
                mt9_binary_image[i][j] = 0;    // 黑色（赛道）

            // 中间（第20-39行）：使用 threshold_middle
            if (mt9_compress_image[i+20][j] >= threshold_middle)
                mt9_binary_image[i+20][j] = 1;
            else
                mt9_binary_image[i+20][j] = 0;

            // 近端（第40-59行）：使用 threshold_near
            if (mt9_compress_image[i+40][j] >= threshold_near)
                mt9_binary_image[i+40][j] = 1;
            else
                mt9_binary_image[i+40][j] = 0;
        }
    }
}
```

**为什么要这样设计？**

在智能车竞赛场景中，摄像头安装在车模上方，向前下方拍摄。图像具有以下光照特征：

| 图像区域 | 行号 | 光照特征 | 阈值调整 | 原因 |
|----------|------|----------|----------|------|
| **远端** | 0-19 | 光线衰减、距离远、整体偏暗 | **-2（降低）** | 降低阈值更容易检测到远处暗部的赛道边界；否则远处赛道会被误判为全黑 |
| **中间** | 20-39 | 过渡区、光照相对均匀 | **-1** | 介于远近之间的平滑过渡 |
| **近端** | 40-59 | 受灯光直射/地面反光最强、整体偏亮 | **+1（提高）** | 提高阈值防止反光区域被误判为白色（赛道外），避免把反光当成赛道丢失 |

**实际场景举例：**

- **强反光地面：** 近端地面反光严重，像素值普遍偏高。如果使用全局阈值，近端反光区域可能被二值化为白色（误判为赛道外），导致小车以为前方没有赛道。分区后近端使用更高阈值（+1），只有真正比反光更亮的背景才会被判定为白色。
- **隧道/阴影：** 远端进入阴影区域，像素值偏低。如果使用全局阈值，远端可能被二值化为全黑（看不到赛道）。分区后远端使用更低阈值（-2），即使整体偏暗也能检测出赛道的灰度差异。

#### 3.4.3 阈值限幅保护

```c
uint8_t OSTU_Low  = 10;    // 阈值下限
uint8_t OSTU_High = 150;   // 阈值上限
```

这两个参数可通过 EEPROM 持久化存储并在菜单中实时调节。它们的作用是：
- **下限 10：** 防止在极暗场景下阈值过低（如阈值为 3），导致大量噪声被误判为赛道
- **上限 150：** 防止在极亮场景下阈值过高（如阈值为 200），导致赛道被误判为背景

### 3.5 步骤7：底边绘制（DrawLinesFirst）

```c
static void DrawLinesFirst(void) {
    PicTemp = mt9_binary_image[59];  // 从最底行（第59行）开始

    // 从右向左扫描，找到右边界
    for (Xsite = 79; Xsite > 0; Xsite--) {
        if (*(PicTemp + Xsite) == 1 && *(PicTemp + Xsite - 1) == 1) {
            BottomBorderRight = Xsite;  // 连续两个白点确认为边界
            break;
        }
        else if (Xsite == 1) {
            BottomBorderRight = 39;     // 找不到时默认为图像中心
            break;
        }
    }

    // 从左向右扫描，找到左边界
    for (Xsite = 0; Xsite < 79; Xsite++) {
        if (*(PicTemp + Xsite) == 1 && *(PicTemp + Xsite + 1) == 1) {
            BottomBorderLeft = Xsite;   // 连续两个白点确认为边界
            break;
        }
        else if (Xsite == 78) {
            BottomBorderLeft = 39;      // 找不到时默认为图像中心
            break;
        }
    }

    BottomCenter = (BottomBorderLeft + BottomBorderRight) / 2;

    // 记录第59行的所有信息
    ImageDeal[59].LeftBorder  = BottomBorderLeft;
    ImageDeal[59].RightBorder = BottomBorderRight;
    ImageDeal[59].Center      = BottomCenter;
    ImageDeal[59].Wide        = BottomBorderRight - BottomBorderLeft;
    ImageDeal[59].IsLeftFind  = 'T';
    ImageDeal[59].IsRightFind = 'T';
}
```

**设计要点：**
- 使用"连续两个白点"作为确认条件，相当于一个简单的**中值滤波**，避免单像素噪声造成的误判
- 找不到边界时默认为图像中心（39），保证算法有合理的回退值
- 底边是整个边沿追踪的**锚点**——所有后续行都依赖第59行的结果

### 3.6 步骤8：自底向上边沿追踪（DrawLinesProcess）—— 核心追踪算法

这是算法中**最长、最复杂的函数**，实现了从底行（第59行）到顶行（OFFLine）的逐行边沿追踪。

```c
static void DrawLinesProcess(void) {
    for (Ysite = 58; Ysite >= ImageStatus.OFFLine; Ysite--) {
        PicTemp = mt9_binary_image[Ysite];

        // === 左侧边沿追踪 ===
        // 搜索范围 = 上一行左边界 ± 3 像素
        IntervalLow  = ImageDeal[Ysite + 1].LeftBorder - 3;
        IntervalHigh = ImageDeal[Ysite + 1].LeftBorder + 3;
        LimitL(IntervalLow);   // 钳制到 [1, 78]
        LimitH(IntervalHigh);
        GetJumpPointFromDet(PicTemp, 'L', IntervalLow, IntervalHigh, &JumpPoint[0]);

        // === 右侧边沿追踪 ===
        IntervalLow  = ImageDeal[Ysite + 1].RightBorder - 3;
        IntervalHigh = ImageDeal[Ysite + 1].RightBorder + 3;
        LimitL(IntervalLow);
        LimitH(IntervalHigh);
        GetJumpPointFromDet(PicTemp, 'R', IntervalLow, IntervalHigh, &JumpPoint[1]);

        // 记录结果
        ImageDeal[Ysite].LeftBorder  = JumpPoint[0].point;
        ImageDeal[Ysite].RightBorder = JumpPoint[1].point;
        ImageDeal[Ysite].IsLeftFind  = JumpPoint[0].type;
        ImageDeal[Ysite].IsRightFind = JumpPoint[1].type;

        // === 如果边沿丢失（'H'或'W'），触发全行重扫描 ===
        if (IsLeftFind == 'H' || IsLeftFind == 'W' || IsRightFind == 'H' || IsRightFind == 'W') {
            // 从图像中间向左右全行扫描，尝试恢复边沿
            // 如果全行扫描也找不到：
            //   - 中间是白色 → 'W'（大弯道/反光导致的白底）
            //   - 中间是黑色 → 'H'（丢线）
            // 如果找到了 → 改为 'T'（恢复成功）
        }

        // === 连续丢线计数 ===
        if (IsLeftFind == 'H' && IsRightFind == 'H')
            continue_lost_count++;
        else
            continue_lost_count = 0;

        // === 计算该行的赛道宽度和中心 ===
        ImageDeal[Ysite].Wide   = RightBorder - LeftBorder;
        ImageDeal[Ysite].Center  = (RightBorder + LeftBorder) / 2;

        // === 提前终止条件 ===
        // 条件1：靠近顶部且连续丢线超过5行 → 提前终止，OFFLine = Ysite + 5
        if (continue_lost_count > 5 && Ysite <= 10) {
            ImageStatus.OFFLine = Ysite + 5;
            break;
        }
        // 条件2：中上部且连续丢线超过25行 → 提前终止，OFFLine = Ysite + 25
        else if (continue_lost_count >= 25 && Ysite <= 35) {
            ImageStatus.OFFLine = Ysite + 25;
            break;
        }
    }
}
```

**`GetJumpPointFromDet()` 函数详细分析：**

这是边沿检测的核心函数，实现了在指定范围内搜索黑白跳变点：

```c
void GetJumpPointFromDet(uint8_t* p, uint8_t type, int L, int H, JumpPointtypedef* JumpPoint) {
    if (type == 'L') {  // 搜索左边界：从左向右扫描
        for (i = L; i <= H; i++) {
            if (*(p + i) == 0 && *(p + i + 1) == 1) {  // 黑→白跳变 = 赛道左边界
                JumpPoint->point = i;
                JumpPoint->type = 'T';  // 找到真实边沿
                break;
            }
            else if (i == (H - 1)) {  // 扫描完整个区间都没找到
                if (*(p + (L + H) / 2) == 1) {  // 区间中间是白色
                    JumpPoint->point = (L + H) / 2;
                    JumpPoint->type = 'W';  // 白色无边（大弯道/反光）
                } else {  // 区间中间是黑色
                    JumpPoint->point = (L + H) / 2;
                    JumpPoint->type = 'H';  // 黑色无边（丢线）
                }
            }
        }
    }
    else if (type == 'R') {  // 搜索右边界：从右向左扫描
        for (i = H; i >= L; i--) {
            if (*(p + i) == 0 && *(p + i - 1) == 1) {  // 白→黑跳变 = 赛道右边界
                JumpPoint->point = i;
                JumpPoint->type = 'T';
                break;
            }
            // ... 同样的 'W'/'H' 判定逻辑
        }
    }
}
```

**为什么这个追踪算法对反光有鲁棒性？**

关键在于**搜索范围受限于上一行的边界位置（±3像素）**。这意味着：

1. 即使某行因为反光导致二值化质量很差，搜索范围也只有7个像素（而非全行80个像素），**反光造成的大面积白色区域不会把边界错误地拉到图像边缘**
2. 赛道边缘在物理上是连续的——相邻两行的边界位置不会跳跃超过3个像素。这个先验知识被编码到了搜索范围中
3. 当确实找不到边沿时，通过判断区间中间的像素颜色来分类为 'W' 或 'H'，交给后续的恢复逻辑处理

**全行重扫描恢复机制：**

当边沿被标记为 'H' 或 'W' 后，代码会触发全行重扫描——从图像中间向左右搜索黑白跳变。这是一种"二次确认"机制：
- 如果在全行范围内找到了新的黑白边界 → 修正为 'T'（恢复成功）
- 如果全行都是黑色 → 保持 'H'（确实丢线）
- 如果全行都是白色 → 保持 'W'（进入大弯道或反光区域，但这是"安全"的丢线——意味着赛道外全白）

### 3.7 步骤9：最长白色连续行计算（Fine_whitepoint）

```c
void Fine_whitepoint(void) {
    uint8_t black_high = 0;
    for (black_high = 2; black_high < 60; black_high++) {
        // 如果该行左右两边都找到了边界（T或W），说明该行以上都是有效赛道区域
        if ((ImageDeal[black_high].IsLeftFind == 'T' || ImageDeal[black_high].IsLeftFind == 'W')
        &&  (ImageDeal[black_high].IsRightFind == 'T' || ImageDeal[black_high].IsRightFind == 'W')) {
            ImageStatus.OFFLine = black_high;  // 更新有效区域的顶边界
            break;
        }
    }
    // 限幅保护
    if (ImageStatus.OFFLine > 58) ImageStatus.OFFLine = 58;
    if (ImageStatus.OFFLine < 2)  ImageStatus.OFFLine = 2;

    whitehigh = 59 - ImageStatus.OFFLine;  // 有效区域的高度（行数）
    whitehigh_float = (float)(whitehigh - 40) / (60 - 40) + 0.5;  // 归一化到约0.5~1.5
}
```

**whitehigh 的作用：**
- 表示从底行向上有多少行是"有效"的（能找到赛道边界的行）
- 光线好、赛道清晰时，whitehigh 较大（接近 57）→ 全图大部分区域都能追踪到
- 光线差、反光严重时，whitehigh 较小（可能只有十几行）→ 只在近端能看清赛道

`whitehigh_float` 被归一化后用于后续的动态前瞻计算。

### 3.8 步骤10：动态前瞻计算（Get_TowPoint）

```c
void Get_TowPoint(void) {
    float WhiteGain = 0;

    // 根据白色有效行数计算前瞻增益
    WhiteGain = 9 * whitehigh_float;  // whitehigh_float 范围约 0.5~1.5

    // 限幅
    if (WhiteGain >= Gain_High) WhiteGain = Gain_High;    // 最大 9
    else if (WhiteGain <= Gain_Low) WhiteGain = Gain_Low; // 最小 -3

    // 计算前瞻点 = 基础前瞻点 - 白色增益
    TowPoint = ImageStatus.TowPoint - WhiteGain;

    // 限幅保护
    if (TowPoint < ImageStatus.OFFLine) TowPoint = ImageStatus.OFFLine + 1;
    if (TowPoint >= 48) TowPoint = 48;
    else if (TowPoint <= 12) TowPoint = 12;

    // 前瞻窗口 = [TowPoint - 10, TowPoint + 10]
    Ysite_bottom = TowPoint - 10;
    Ysite_top    = TowPoint + 10;
}
```

**动态前瞻的物理意义：**

| 场景 | whitehigh | WhiteGain | TowPoint | 含义 |
|------|-----------|-----------|----------|------|
| 光线好、赛道清晰 | 大（接近57） | 大（接近9） | TowPoint - 9（更近） | 赛道清晰，不需要看太远，专注近处精准控制 |
| 光线差、反光多 | 小（约20） | 小（接近-3） | TowPoint + 3（更远） | 近处反光看不清，需要看得更远来预判赛道走向 |

`ImageStatus.TowPoint` 的默认值为 18，可通过菜单实时调节。

**注意：** 这里的逻辑是 `TowPoint = ImageStatus.TowPoint - WhiteGain`。当 WhiteGain 大（赛道清晰）时，TowPoint 减小（前瞻更近）；当 WhiteGain 小（赛道不清）时，TowPoint 增大（前瞻更远）。这是一种**自适应的前瞻策略**——赛道信息可靠时专注于近处精度，赛道信息不可靠时看远处获取更多上下文。

### 3.9 步骤11：拐点搜索（Find_Spinodal）

```c
void Find_Spinodal(void) {
    int center_count = 0;

    for (int i = 2; i <= 57; i++) {
        // 左拐点：最左侧列(第1列)从黑变白的位置
        if (mt9_binary_image[57-i+2][1] == 1
            && ImageDeal[57-i+2].IsLeftFind == 'W'
            && ImageFrameDetect.spinodal_L_Ysite == 0) {
            ImageFrameDetect.spinodal_L_Ysite = 57 - i + 2;
        }

        // 右拐点：最右侧列(第78列)从黑变白的位置
        if (mt9_binary_image[57-i+2][78] == 1
            && ImageDeal[57-i+2].IsRightFind == 'W'
            && ImageFrameDetect.spinodal_R_Ysite == 0) {
            ImageFrameDetect.spinodal_R_Ysite = 57 - i + 2;
        }

        // 左侧第一个 W→T 转变点
        if (ImageDeal[i].IsLeftFind == 'W' && ImageDeal[i+1].IsLeftFind == 'T')
            ImageStatus.First_W_Point_L = i+1;
        // 右侧第一个 W→T 转变点
        if (ImageDeal[i].IsRightFind == 'W' && ImageDeal[i+1].IsRightFind == 'T')
            ImageStatus.First_W_Point_R = i+1;
        // 左侧第一个 H→T/W 转变点
        if (ImageDeal[i].IsLeftFind == 'H'
            && (ImageDeal[i+1].IsLeftFind == 'T' || ImageDeal[i+1].IsLeftFind == 'W'))
            ImageStatus.First_H_Point_L = i+1;
        // 右侧第一个 H→T/W 转变点
        if (ImageDeal[i].IsRightFind == 'H'
            && (ImageDeal[i+1].IsRightFind == 'T' || ImageDeal[i+1].IsRightFind == 'W'))
            ImageStatus.First_H_Point_R = i+1;

        // 所有拐点特征都找到后提前退出
        if (所有特征都已找到) break;
    }

    // 计算图像中心（取底边10行的中心线平均值）
    for (i = 59; i >= ImageStatus.OFFLine; i--) {
        if (ImageDeal[i].IsLeftFind == 'T' && ImageDeal[i].IsRightFind == 'T') {
            ImageFrameDetect.image_center_x += ImageDeal[i].Center;
            center_count++;
        }
    }
    ImageFrameDetect.image_center_x /= center_count;
}
```

拐点（Spinodal Point）是赛道元素识别的关键特征。具体含义：
- **左拐点** (`spinodal_L_Ysite`)：图像最左侧（第1列）从黑色变为白色的行号 → 表示赛道左边界在此行消失（可能进入弯道或路口）
- **右拐点** (`spinodal_R_Ysite`)：图像最右侧（第78列）从黑色变为白色的行号 → 表示赛道右边界在此行消失
- **First_W_Point_L/R**：边沿类型从 'W' 变为 'T' 的行号 → 无边界变为有边界的转折点
- **First_H_Point_L/R**：边沿类型从 'H' 变为 'T' 或 'W' 的行号 → 黑色无边变为有边界的转折点

### 3.10 步骤12：赛道元素判断（Element_Judgment）

基于前面提取的拐点特征，`Element_Judgment()` 函数识别 7 种赛道元素：

```c
typedef enum {
    Normol,          // 0 - 正常赛道（无元素）
    RightAngle_L,    // 1 - 左直角转弯
    RightAngle_R,    // 2 - 右直角转弯
    Node_L,          // 3 - 左节点（左支路）
    Node_R,          // 4 - 右节点（右支路）
    Node_T,          // 5 - T型节点（三岔路口）
    Branch,          // 6 - 分叉路
    Cross,           // 7 - 十字路口
    Correct,         // 8 - 补线（encoder到达目标距离后的修正）
} RoadType_e;
```

每种元素的识别逻辑基于**拐点特征组合 + 框检测（Frame_Detect）**：

#### 框检测（Frame_Detect）原理

```c
void Frame_Detect(int center_x, int center_y, int frame_width, int frame_height) {
    int Y_bottom = center_y + frame_height/2;
    int Y_top    = center_y - frame_height/2;
    int X_right  = center_x + frame_width/2;
    int X_left   = center_x - frame_width/2;

    // 沿垂直方向扫描框的左右边界
    for (Ysite = Y_top; Ysite <= Y_bottom-2; Ysite++) {
        // 检测黑白跳变次数（flag: 0→1→2→3→4）
        // flag = 0: 初始状态
        // flag = 1: 第一次黑→白跳变
        // flag = 2: 第一次白→黑跳变
        // flag = 3: 第二次黑→白跳变
        // flag = 4: 第二次白→黑跳变
    }

    // 沿水平方向扫描框的上下边界
    for (Xsite = X_left; Xsite <= X_right-2; Xsite++) {
        // 同样的黑白跳变计数
    }
}
```

框检测通过在指定矩形区域的四条边上扫描黑白交替（0→1→0→1→0）的次数来判断该区域内是否存在赛道特征。例如：
- `frame_top_flag == 0 && frame_bottom_flag == 2`: 上边无边，下边有完整的黑白交替 → 下方有赛道
- `frame_left_flag == 2 && frame_right_flag == 0`: 左边有完整黑白交替，右边没有 → 赛道靠左

#### 7种元素的识别条件

| 元素 | spinodal_L | spinodal_R | First_W_L | First_W_R | First_H_L | First_H_R | 宽度条件 | 框检测条件 |
|------|-----------|-----------|-----------|-----------|-----------|-----------|---------|-----------|
| **左直角** | ≠0 | =0 | ≠0 | =0 | ≠0 | ≠0 | 宽度正常 | top=0, bottom=2, left=2, right=0 |
| **右直角** | =0 | ≠0 | =0 | ≠0 | ≠0 | ≠0 | 宽度正常 | top=0, bottom=2, left=0, right=2 |
| **左节点** | ≠0 | =0 | ≠0 | =0 | =0 | =0 | 宽度正常 | top=2, bottom=2, left=2, right=0 |
| **右节点** | =0 | ≠0 | =0 | ≠0 | =0 | =0 | 宽度正常 | top=2, bottom=2, left=0, right=2 |
| **T节点** | ≠0 | ≠0 | ≠0 | ≠0 | ≠0 | ≠0 | 宽度正常 | top=0, bottom=2, left=2, right=2 |
| **分叉路** | ≠0 | ≠0 | ≠0 | ≠0 | ≠0 | ≠0 | 宽度>15（异常宽） | top=0, bottom=2, left=2, right=2 |
| **十字路** | ≠0 | ≠0 | ≠0 | ≠0 | =0 | =0 | 宽度正常 | top=2, bottom=2, left=2, right=2 |

此外，所有元素识别都需要满足：
- 拐点位置在合理范围内（`Y_top_limit` ~ `Y_bottom_limit`）
- 编码器里程达到一定距离（`encoder.disA >= target_distance * distance_judgment`），防止重复识别
- 当前道路状态为 Normol（正常行驶中）

### 3.11 步骤13：路径滤波（RouteFilter）

```c
static void RouteFilter(void) {
    if (ImageStatus.Road_type == Normol) {
        for (Ysite = 58; Ysite >= Ysite_bottom; Ysite--) {
            // 如果该行有无边标记（W或H），用图像中心替代
            if (ImageDeal[Ysite].IsLeftFind == 'W' || ImageDeal[Ysite].IsLeftFind == 'H'
                || ImageDeal[Ysite].IsRightFind == 'W' || ImageDeal[Ysite].IsRightFind == 'H') {
                ImageDeal[Ysite].Center = ImageFrameDetect.image_center_x;
            }
        }
    }
}
```

**作用：** 当某些行因为反光或丢线导致边沿异常时，用图像全局中心线替代该行的中心值，避免异常值干扰转向控制。这是一种简单的**异常值平滑处理**。

### 3.12 步骤14：元素处理（Element_Handle）

根据识别的赛道元素类型，调用对应的处理函数。每个处理函数的核心逻辑：
1. 打开蜂鸣器（`buzzer_on()`）
2. 设置目标转向角度
3. 设置目标行驶距离
4. 将该元素区域内的中心线强制设置为赛道某一侧（让小车"看不到"赛道另一侧，从而执行转向）
5. 转向完成后，清除标志位，切换到下一段路径

以左直角转弯为例：

```c
void Elment_Handle_RightAngle_L(void) {
    buzzer_on();
    post.target_angle = Path[...].memory_target_angle[...];
    encoder.target_distance = Path[...].memory_target_distance[...];

    // 将 First_W_Point_L 以上所有行的中心线设为 0（强制靠左）
    for (Ysite = 0; Ysite <= ImageStatus.First_W_Point_L; Ysite++) {
        ImageDeal[Ysite].Center = 0;
    }

    // 状态机：1→识别, 2→执行中, 3→完成
    if (ImageFlag.image_element_angle_flag == 1)
        ImageFlag.image_element_angle_flag = 2;

    // 角度到位后清除
    if (abs(post.diff_angle) < 10 && ImageFlag.image_element_angle_flag == 3) {
        buzzer_off();
        encoder_clear();
        ImageStatus.Road_type = Normol;
        ImageFlag.image_element_angle_flag = 0;
        Path[...].target_angle_index++;
    }
}
```

### 3.13 步骤15：加权偏差计算（Get_Deviation）

```c
// 20个点的类高斯分布权重（中心对称）
float Weighting_20[20] = {
    0.020, 0.032, 0.050, 0.076, 0.112,   // 0-4: 边缘权重逐渐增加
    0.158, 0.215, 0.284, 0.364, 0.450,   // 5-9: 中心区域权重快速增加
    0.450, 0.364, 0.284, 0.215, 0.158,   // 10-14: 对称下降
    0.112, 0.076, 0.050, 0.032, 0.020    // 15-19: 边缘权重逐渐减小
};

void Get_Deviation(void) {
    float UnitAll = 0;
    // 对前瞻窗口内的20行做加权平均
    for (int Ysite = Ysite_bottom; Ysite < Ysite_top; Ysite++) {
        DetTemp += Weighting_20[Ysite - Ysite_bottom]
                 * ImageDeal[Ysite].Center;
        UnitAll += Weighting_20[Ysite - Ysite_bottom];
    }
    // 再加上前瞻点本身的中心值（权重=1）
    DetTemp = (ImageDeal[TowPoint].Center + DetTemp) / (UnitAll + 1);

    ImageStatus.Det_True = DetTemp;       // 最终的加权图像偏差
    ImageStatus.TowPoint_True = TowPoint; // 实际使用的前瞻点
}
```

**加权策略分析：**

这20个权重值服从**高斯分布（正态分布）**形状：
- 中间行（索引9-10）权重最高为 0.450
- 边缘行（索引0-1和18-19）权重最低为 0.020~0.032
- 中心行权重是边缘行的 **22.5 倍**

这种加权方式的意义：
- **前瞻点附近的行**对转向决策影响最大——它们最准确地反映了当前赛道走向
- **距离前瞻点较远的行**影响较小——它们反映的是更远/更近的赛道状态，对当前转向决策的参考价值较低
- 加权平均能有效**平滑**因个别行二值化异常（反光导致）引起的中心线跳动

---

## 四、光照鲁棒性技术总结

### 4.1 技术栈总览

| 层次 | 技术 | 解决的问题 | 核心参数 |
|------|------|-----------|---------|
| **像素级** | Gamma 校正 | 强光下对比度不足，赛道被"冲淡" | gamma = 0.5 |
| **阈值级** | 分区 OTSU | 光照不均匀（远处暗、近处亮） | 三区偏移: -2, -1, +1 |
| **阈值级** | 阈值限幅 | 极端光照下的阈值崩溃 | [10, 150] |
| **边沿级** | 邻行约束搜索 | 反光引起的局部二值化错误 | 搜索半径 = ±3 |
| **边沿级** | 三态分类 + 全行重扫 | 边沿丢失后的恢复 | 'T'/'W'/'H' |
| **行级** | 连续丢线计数 | 判断跟踪是否真正失败 | 阈值: 5行/25行 |
| **帧级** | 动态前瞻 | 自适应调整观察距离 | TowPoint = 18 - WhiteGain |
| **帧级** | 高斯加权 | 平滑异常行的影响 | 中心权重 0.450 |
| **帧级** | 路径滤波 | 用全局中心替代异常行的中心 | RouteFilter |

### 4.2 针对不同光照场景的鲁棒性分析

#### 场景一：强光直射（晴天中午，阳光直射赛道）

**问题表现：** 图像整体过亮，赛道和背景的灰度差减小，全局OTSU可能产生过高的阈值导致赛道被误判为背景。

**应对措施：**
1. **Gamma = 0.5**：压缩亮部，拉伸暗部，恢复被强光冲淡的对比度
2. **分区阈值**：近端使用 threshold_total + 1，进一步避免过曝区域的误判
3. **阈值上限 150**：即使 OTSU 算出阈值 200，也被钳制到 150

#### 场景二：地面反光（光滑地面反射灯光）

**问题表现：** 近端地面出现大面积高亮反光区域，二值化后这些区域变成白色，赛道被"淹没"。

**应对措施：**
1. **近端阈值 +1**：反光区域像素值虽然高，但比真正的背景（赛道外）仍然略低，提高阈值可以正确区隔
2. **边沿追踪搜索范围 ±3**：即使某行大部分被反光覆盖，搜索范围限制在上一行边沿附近 ±3 像素，不会把边界错误地拉到反光区域
3. **三态分类**：如果反光导致某行全是白色，标记为 'W'（白底无边），触发全行重扫恢复，而非直接判定为丢线
4. **高斯加权**：个别反光行的异常中心值被低权重平滑

#### 场景三：光照不均匀（隧道出口/室内灯光渐变）

**问题表现：** 图像一边亮一边暗，或远处暗近处亮，全局OTSU无法适应。

**应对措施：**
1. **分区 OTSU（核心）**：远中近三区使用不同阈值，这是针对此问题的直接解决方案
2. **动态前瞻**：远处太暗看不清时，前瞻点自动后移，专注于近处可靠的数据

#### 场景四：阴影遮挡（树木阴影、建筑物阴影）

**问题表现：** 赛道被阴影分割成明暗区域，全局OTSU可能在阴影边界产生错误分割。

**应对措施：**
1. **边沿追踪的连续性约束**：阴影边界通常不是赛道边界，但由于边沿追踪只搜索上一行附近 ±3 范围，阴影边界如果与赛道边界不连续，就不会被误追踪
2. **阈值下限 10**：防止阴影区域的 OTSU 阈值过低导致噪声被放大
3. **连续丢线计数**：短暂被阴影遮挡不会立即触发丢线

### 4.3 参数可调性

整套算法有多个可通过菜单实时调节的参数，存储在 EEPROM 中，断电不丢失：

| 参数 | 默认值 | 范围 | 作用 |
|------|--------|------|------|
| `gamma_value` | 0.5 | 0~2.0 | Gamma 校正强度 |
| `OSTU_High` | 150 | 0~255 | OTSU 阈值上限 |
| `OSTU_Low` | 10 | 0~255 | OTSU 阈值下限 |
| `TowPoint` | 18 | - | 基础前瞻点位置 |
| `Gain_High` | 9 | - | 动态前瞻增益上限 |
| `Gain_Low` | -3 | - | 动态前瞻增益下限 |
| `ImageScanInterval_small` | 3 | - | 边沿搜索范围半径 |

### 4.4 算法的局限性与改进空间

**当前局限性：**

1. **分区数固定为3**：只有远中近三个分区，对于更复杂的光照分布（如斜射光导致左右光照不均）可能不够
2. **分区阈值偏移量固定**：-2/-1/+1 是经验值，极端光照下可能需要更大的偏移
3. **没有使用自适应 Gamma**：gamma 值是固定的 0.5，不会根据图像亮度自动调节
4. **最近邻降采样**：2:1 降采样使用最近邻而非双线性插值，可能丢失细边缘信息

**潜在的改进方向：**

1. 可以在 OTSU 之前对图像做**光照补偿**（如顶帽变换、同态滤波）
2. 可以将 gamma 值改为根据图像平均亮度**自适应计算**
3. 可以在边沿追踪中加入**卡尔曼滤波**，进一步平滑边沿轨迹
4. 可以使用**自适应分区**——根据光照分布自动确定分区位置和数量

---

## 五、代码架构与数据结构

### 5.1 全局结构体

```c
// 每行的赛道信息
typedef struct {
    uint8_t IsRightFind;     // 右边界查找状态: 'T'/'W'/'H'/'F'
    uint8_t IsLeftFind;      // 左边界查找状态
    int Wide;                // 赛道宽度
    int LeftBorder;          // 左边界 X 坐标
    int RightBorder;         // 右边界 X 坐标
    int LeftBorder2;         // 修正后的左边界
    int RightBorder2;        // 修正后的右边界
    int Center;              // 中心线 X 坐标
} ImageDealDatatypedef;

// 全局图像状态
typedef struct {
    int TowPoint;            // 设定的前瞻点
    int TowPoint_True;       // 实际使用的前瞻点（动态调整后）
    int Det_True;            // 加权偏差（最终输出给转向控制）
    uint8_t OFFLine_Last;    // 上一帧的有效顶边界
    uint8_t OFFLine;         // 当前帧的有效顶边界
    RoadType_e Road_type;    // 当前赛道元素类型

    int First_W_Point_L;     // 左侧第一个白色无边点行号
    int First_W_Point_R;     // 右侧第一个白色无边点行号
    int First_H_Point_L;     // 左侧第一个黑色无边点行号
    int First_H_Point_R;     // 右侧第一个黑色无边点行号
} ImageStatustypedef;

// 框检测结果
typedef struct {
    int spinodal_L_Ysite;    // 左拐点行号
    int spinodal_R_Ysite;    // 右拐点行号
    int image_center_x;      // 图像全局中心 X
    int frame_center_x/y;    // 检测框中心坐标
    int frame_width/height;  // 检测框尺寸
    int frame_bottom_flag;   // 下边界交替次数
    int frame_right_flag;    // 右边界交替次数
    int frame_top_flag;      // 上边界交替次数
    int frame_left_flag;     // 左边界交替次数
} ImageFrameDetecttypedef;

// 赛道元素锁定（允许/禁止识别）
typedef struct {
    uint8_t RightAngle_L;    // 左直角
    uint8_t RightAngle_R;    // 右直角
    uint8_t Node_L;          // 左节点
    uint8_t Node_R;          // 右节点
    uint8_t Node_T;          // T节点
    uint8_t Branch;          // 分叉路
    uint8_t Cross;           // 十字路
    uint8_t Correct;         // 补线
    uint8_t Protect;         // 保护（检测到全黑时停车）
} RoadType_lock;

// 识别标志位
typedef struct {
    uint8_t image_element_Node_flag;     // 节点识别状态: 0未/1已/2执行/3完成
    uint8_t image_element_angle_flag;    // 直角识别状态
    uint8_t image_element_branch_flag;   // 分叉路识别状态
    uint8_t image_element_cross_flag;    // 十字路识别状态
    uint8_t image_element_correct_flag;  // 补线状态
} ImageFlagtypedef;
```

### 5.2 状态机设计

每个赛道元素的处理都遵循**四级状态机**：

```
Flag = 0  →  未识别（等待触发条件）
Flag = 1  →  已识别（Element_Judgment 满足条件，设置标志）
Flag = 2  →  执行中（Element_Handle 开始执行转向/直行动作）
Flag = 3  →  完成（转向到位或行驶距离达标，清除标志回到0）
```

### 5.3 安全保护机制

```c
void Protect_Car(void) {
    // 当有效区域几乎消失（OFFLine >= 55）
    // 且第55行的中间、1/3、2/3位置全部是黑色时
    // 触发紧急停车
    if (ImageStatus.OFFLine >= 55
        && mt9_binary_image[55][40] == 0       // 中间
        && mt9_binary_image[55][26] == 0       // 1/3处
        && mt9_binary_image[55][53] == 0) {    // 2/3处
        stop_flag = 1;  // 紧急停车
    }
}
```

当赛道信息极度不可靠（有效区域只剩不足5行）且多个采样点都是黑色时，触发紧急停车，防止小车冲出赛道。

---

## 六、总结

"上帝之光"（Eye of Providence）是一套为全国大学生智能车竞赛设计的灰度摄像头图像处理算法，运行在 Infineon TC264D 平台上。它使用 MT9V03X 120×160 灰度摄像头，将图像压缩为 80×60 后进行处理。

算法名称借用了"上帝全视之眼"的意象，表达其在各种光照条件下"看穿一切"的设计追求。

**核心创新点：**

1. **分区大津法二值化** — 将图像分为远/中/近三个区域，分别使用不同的二值化阈值（-2/-1/+1 偏移），直接解决了光照不均匀导致的全局阈值失效问题。这是整套算法对光线鲁棒性最根本的保障。

2. **Gamma 0.5 校正** — 在强光场景下增强图像对比度，恢复被"冲淡"的赛道边缘信息。

3. **基于邻行约束的边沿追踪** — 搜索范围限定在上一行边界 ±3 像素，利用赛道边缘的物理连续性，有效抵抗反光造成的局部二值化异常。

4. **三态边沿分类与恢复机制** — 'T'（真边）、'W'（白底无边）、'H'（黑底无边）的三态分类 + 全行重扫描恢复，使算法能够在边沿丢失时采取不同的恢复策略。

5. **动态前瞻调整** — 根据赛道可视程度自动调整前瞻距离，清晰时专注近处精度，模糊时看远处获取更多上下文。

6. **高斯加权偏差计算** — 通过类正态分布的权重平滑异常行的中心线跳动，使转向控制更加稳定。

整套算法在强光、反光、光照不均匀等场景下表现出良好的鲁棒性，通过多层防御机制（像素级→阈值级→边沿级→行级→帧级）层层递进地对抗光照干扰。配合作者精细的参数调节，在省级智能车竞赛中取得了第一名的好成绩。
