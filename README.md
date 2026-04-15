# ConfigStudio

> 基于 **Qt Widgets + Qwt** 的工业 HMI 组态编辑与运行预览工具。

ConfigStudio 是一个面向工业触控屏/工控机场景的可视化组态应用。它提供了从 **组件拖拽布局**、**属性编辑**、**变量绑定** 到 **运行态预览** 的完整链路，适合用于设备面板、人机界面、产线看板等项目的快速构建与联调。

---

## 目录

- [项目简介](#项目简介)
- [功能特性](#功能特性)
- [界面与工作流](#界面与工作流)
- [技术架构](#技术架构)
- [目录结构](#目录结构)
- [环境依赖](#环境依赖)
- [构建与运行](#构建与运行)
- [测试](#测试)
- [组件开发指南（简版）](#组件开发指南简版)
- [嵌入式/触屏注意事项](#嵌入式触屏注意事项)
- [项目现状与后续规划](#项目现状与后续规划)
- [文档索引](#文档索引)

---

## 项目简介

ConfigStudio 的核心定位：

1. **设计态（Design Mode）**：用于页面搭建、组件摆放、属性配置；
2. **运行态（Runtime Mode）**：用于联调与运行预览，降低误触导致布局变动风险；
3. **变量驱动**：通过 `varId` 将数据模型中的变量值实时映射到控件显示/行为；
4. **触屏友好**：重点优化在嵌入式 Linux + Wayland/X11 环境下的输入体验与稳定性。

本项目尤其关注工业场景的“可用性”与“可维护性”：

- 统一的组件属性接口；
- 清晰的数据流分层；
- 支持本地仿真与串口/Modbus RTU 数据源接入。

---

## 功能特性

### 1) 可视化画布编辑

- 拖拽添加组件到画布；
- 组件选中、移动、缩放、删除；
- 属性面板实时显示当前选中组件属性。

### 2) 丰富的工业组件

内置组件（持续扩展中）包括：

- `plot`（曲线图）
- `indicator`（状态指示灯）
- `text`（文本显示）
- `numeric`（数值显示）
- `dial`、`thermo`、`histogram`、`slider`、`wheel`、`switch`

### 3) 属性系统

- 组件通过统一接口 `properties()` 暴露可编辑属性；
- 属性改动通过 `setPropertyValue(key, value)` 回写到组件；
- 布尔/枚举/颜色等属性支持快捷切换，文本与数值可通过内嵌输入面板编辑。

### 4) 数据绑定与数据源

- `DataBindingManager` 负责变量更新到组件属性的分发；
- `RuntimeSimulator` 支持无 PLC 场景下的本地仿真；
- 提供串口/Modbus RTU 数据源能力，支持运行态联调。

### 5) 运行安全与稳定性设计

- 运行态锁定编辑行为，避免误改布局；
- 针对输入法/弹层生命周期做了嵌入式适配；
- 退出流程采用应用内确认面板并执行安全收尾（输入法收敛、轮询停止、数据源关闭等）。

---

## 界面与工作流

典型使用步骤：

1. 启动应用，进入设计态；
2. 从组件区选择控件并放置到画布；
3. 在右侧属性面板中设置样式、行为与 `varId`；
4. 切换到运行态，观察变量驱动后的动态效果；
5. 根据需要切回设计态继续调优。

典型数据流：

1. 用户选中画布中的组件；
2. 主窗口读取该组件 `properties()` 渲染属性表；
3. 用户编辑属性；
4. 主窗口调用 `setPropertyValue()` 回写；
5. 若为绑定属性，则由运行层把变量值持续推送到组件。

---

## 技术架构

项目整体可理解为 6 层：

- **UI 层**：主窗口、属性面板、弹层与交互协调；
- **Canvas 层**：画布与可编辑图元抽象；
- **Component 层**：具体工业控件实现；
- **Model 层**：变量模型及工程数据承载；
- **Runtime 层**：运行态窗口、绑定分发、仿真逻辑；
- **DataSource 层**：串口/Modbus RTU 等外部数据接入。

这种分层方式可降低耦合，便于后续扩展新组件、新协议与新运行机制。

---

## 目录结构

```text
ConfigStudio/
├── app/            # 应用上下文、模式枚举、全局配置
├── ui/             # MainWindow 及交互编排
├── canvas/         # CanvasView / CanvasItem 与编辑行为
├── components/     # 组件实现（plot/text/indicator/...）
├── model/          # 变量/页面等模型
├── runtime/        # 运行态窗口、绑定管理、仿真
├── datasrc/        # 串口与 Modbus RTU 数据源
├── commands/       # 命令基础设施（可用于撤销重做扩展）
├── virtualkey/     # 软键盘与触控输入支持
├── resources/      # 图标、资源配置
├── tests/          # Qt Test 测试
└── docs/           # 开发与测试文档
```

---

## 环境依赖

推荐环境：Linux（X11 或 Wayland）

- Qt 5.x（建议 5.12+）
  - 主要模块：`core/gui/widgets/quick/quickwidgets/qml/serialport`
- Qwt
- qmake + make

Ubuntu 参考安装：

```bash
sudo apt update
sudo apt install -y \
  qtbase5-dev qtdeclarative5-dev qttools5-dev qttools5-dev-tools \
  libqt5serialport5-dev libqwt-qt5-dev build-essential
```

---

## 构建与运行

### 1) 构建

```bash
qmake ConfigStudio.pro
make -j$(nproc)
```

### 2) 运行

```bash
./ConfigStudio
```

### 3) 启用属性诊断日志（可选）

```bash
CONFIGSTUDIO_PROP_DIAG=1 ./ConfigStudio
```

---

## 测试

当前测试框架为 **Qt Test**，可执行模型层测试：

```bash
qmake tests/variablemodel_test.pro -o tests/Makefile
make -C tests -j$(nproc)
./tests/variablemodel_test
```

若出现常见错误：

- `qmake: command not found`：缺少 Qt 构建工具；
- `-lqwt` 链接失败：Qwt 开发包未安装或库路径未配置。

---

## 组件开发指南（简版）

新增组件建议遵循以下约定：

1. 提供唯一 `type()` 标识；
2. 在 `properties()` 中完整暴露可编辑属性；
3. 在 `setPropertyValue()` 中做类型转换与容错；
4. 明确哪些属性可绑定变量（如 `varId`）；
5. 属性变更后及时刷新显示（重绘/更新）；
6. 遵循运行态编辑锁定规则。

---

## 嵌入式/触屏注意事项

为提升在嵌入式图形栈中的稳定性，建议：

- 优先使用应用内嵌弹层/输入面板；
- 谨慎使用系统原生模态对话框；
- 控制输入法焦点切换与临时窗口生命周期；
- 对高频更新组件（如 plot）保持轻量刷新策略。

---

## 项目现状与后续规划

### 当前现状

- 已具备完整的设计态/运行态闭环；
- 组件体系可用并可扩展；
- 变量绑定、仿真与 Modbus RTU 接入已打通基础链路。

### 后续规划

- 工程文件持久化（布局+属性+绑定）；
- 撤销/重做命令栈完善；
- 更多工业协议接入（如 OPC UA / MQTT）；
- 告警体系、权限体系与主题模板。

---

## 文档索引

- 开发与测试指南：[`docs/DEVELOPMENT.md`](docs/DEVELOPMENT.md)

如果你准备进行二次开发，建议先阅读上述文档再进入代码实现。
