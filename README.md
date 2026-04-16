# ConfigStudio

> 基于 **Qt Widgets + Qwt** 的工业 HMI 组态编辑与运行预览工具。

## 项目简介

ConfigStudio 面向工业触控屏/工控机场景，提供从页面设计到运行预览的完整链路：

- **设计态（Design Mode）**：拖拽组件、编辑属性、绑定变量。
- **运行态（Runtime Mode）**：锁定编辑行为，进行联调与运行验证。
- **数据驱动**：通过 `varId` 将变量模型实时映射到组件属性。

## 功能特性

- 画布编辑：组件添加、移动、缩放、删除。
- 属性系统：统一 `properties()` / `setPropertyValue()` 接口。
- 数据源支持：本地仿真 + 串口 / Modbus RTU。
- 工程管理：工程保存、加载、删除、重命名。
- 触控优化：内嵌输入面板、运行态安全退出流程。

## 当前架构

```text
ConfigStudio/
├── app/            # 应用级配置与模式定义
├── ui/             # MainWindow 与界面交互编排
├── canvas/         # CanvasView / CanvasItem
├── components/     # 各类工业组件实现
├── model/          # 变量模型与工程文件读写
├── runtime/        # 运行态窗口、绑定管理、仿真
├── datasrc/        # 串口与 Modbus RTU 数据源
├── commands/       # 命令基础设施
├── virtualkey/     # 软键盘与触控输入
├── resources/      # 图标与 qrc 资源
└── docs/           # 开发文档
```

> 已移除仓库内未接入主工程的模板代码、空壳类及历史测试样例，保留与主应用直接相关的模块，降低维护噪音与耦合成本。

## 环境依赖

推荐 Linux（X11 或 Wayland）环境：

- Qt 5.12+
  - `core gui widgets quick quickwidgets qml serialport`
- Qwt
- qmake + make

Ubuntu 示例：

```bash
sudo apt update
sudo apt install -y \
  qtbase5-dev qtdeclarative5-dev qttools5-dev qttools5-dev-tools \
  libqt5serialport5-dev libqwt-qt5-dev build-essential
```

## 构建与运行

```bash
qmake ConfigStudio.pro
make -j$(nproc)
./ConfigStudio
```

可选：启用属性诊断日志

```bash
CONFIGSTUDIO_PROP_DIAG=1 ./ConfigStudio
```

## 文档

- 开发说明：[`docs/DEVELOPMENT.md`](docs/DEVELOPMENT.md)
