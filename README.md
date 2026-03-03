# ConfigStudio

ConfigStudio 是一个基于 **Qt Widgets + Qwt** 的工业组态/可视化编辑工具。它提供了“设计态 + 运行态”双模式：
在设计态中可拖拽组件、配置属性和绑定变量；在运行态中可进行全屏观察与交互展示。

---

## 1. 项目定位

ConfigStudio 面向 HMI/工业看板场景，核心目标是：

- 以可视化方式快速搭建监控界面；
- 将组件属性与变量数据源绑定；
- 在运行态实时渲染曲线、数值、状态灯等控件；
- 支持触屏友好的属性编辑与全屏预览。

---

## 2. 核心能力

### 2.1 画布设计

- 在画布中添加/选择组件；
- 支持组件移动、缩放（设计态）；
- 属性面板动态展示当前组件可编辑参数。

### 2.2 组件体系

项目内置多种常见可视化组件，例如：

- `plot`（曲线图）
- `dial`（仪表盘）
- `thermo`（温度条）
- `slider`
- `wheel`
- `switch`
- `indicator`（指示灯）
- `numeric`
- `text`
- `histogram`

### 2.3 数据绑定与仿真

- 组件可通过 `varId` 与变量模型绑定；
- 运行时由绑定管理器分发变量更新；
- 内置仿真器可生成周期/随机数据，便于离线调试界面效果。

### 2.4 运行态预览

- 支持全屏运行窗口；
- 运行态与设计态隔离（避免误操作改动布局）；
- 对可交互控件（如 slider/switch/wheel）保留运行态交互能力。

### 2.5 触屏友好的属性编辑

- 常用布尔属性支持点击切换；
- `mode` 支持点击在 `above/below` 间切换；
- 颜色属性支持点击轮替（如 gray/red/green/blue）；
- 属性顺序做了优先级排序，常用项更靠前。

---

## 3. 目录结构说明

```text
ConfigStudio/
├─ app/                # 应用入口/应用层封装
├─ canvas/             # 画布与基础图元（CanvasView/CanvasItem）
├─ commands/           # 命令相关（如撤销重做可扩展点）
├─ components/         # 各类业务组件（plot/indicator/...）
├─ datasrc/            # 数据源相关界面或适配逻辑
├─ fullscreen/         # 全屏视图辅助模块
├─ model/              # 变量模型、数据结构
├─ resources/          # 图标等资源文件
├─ runtime/            # 运行态窗口、绑定管理、仿真器
├─ ui/                 # 主界面与属性编辑交互
├─ utils/              # 通用工具
├─ virtualkey/         # 虚拟键盘相关
├─ main.cpp            # 程序入口
└─ ConfigStudio.pro    # qmake 工程文件
```

---

## 4. 关键模块简介

### `canvas/`

- `CanvasView`：承载组件、处理拖拽投放与选中逻辑；
- `CanvasItem`：组件基类，统一属性接口与交互行为（拖拽/缩放/选中）。

### `components/`

每个组件通常实现：

- `type()`：组件类型标识；
- `properties()`：向属性面板暴露可编辑属性；
- `setPropertyValue()`：属性回写入口；
- 组件自身绘制/布局/数据刷新逻辑。

### `runtime/`

- `RuntimeWindow`：运行态全屏窗口；
- `DataBindingManager`：变量到组件属性的绑定分发；
- `RuntimeSimulator`：测试数据驱动。

### `ui/`

- `MainWindow`：主工作区、组件面板、属性面板、模式切换；
- 属性表支持触屏优化与快捷切换逻辑。

---

## 5. 构建与运行

### 5.1 环境依赖

- Qt（Widgets 模块）
- qmake
- Qwt
- Linux/嵌入式图形环境（X11 或 Wayland）

### 5.2 构建命令

```bash
qmake ConfigStudio.pro
make -j
```

### 5.3 运行

```bash
./ConfigStudio
```

---

## 6. 调试建议

### 6.1 属性流转诊断日志

项目支持通过环境变量开启部分诊断日志：

```bash
CONFIGSTUDIO_PROP_DIAG=1 ./ConfigStudio
```

可用于排查属性面板点击、属性回写、组件更新等时序问题。

### 6.2 Wayland 场景建议

在嵌入式 Wayland 环境中，建议：

- 避免频繁创建/销毁复杂弹窗；
- 优先采用主界面内嵌交互面板；
- 降低输入法焦点切换与控件重建的重叠概率。

---

## 7. 后续可扩展方向

- 工程保存/加载（布局与绑定配置持久化）
- 撤销/重做命令栈
- 更丰富的数据源接入（PLC/OPC UA/MQTT 等）
- 运行态权限控制与告警系统
- 组件主题化（深浅色、工业配色模板）

---

## 8. 许可与说明

本 README 主要用于工程结构与使用方式说明，具体许可与发布策略请结合项目实际管理规范补充。
