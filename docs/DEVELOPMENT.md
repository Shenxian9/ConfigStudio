# ConfigStudio 开发指南

> 本文面向二次开发者，给出从环境准备、编译运行到排障的一套可落地流程。

---

## 1. 开发环境要求

### 1.1 必备依赖

- **Qt 5.x（推荐 5.12+）**
  - 模块：`core`、`gui`、`widgets`、`quick`、`quickwidgets`、`qml`、`serialport`
- **Qwt**（用于工业图表组件）
- **qmake + make**（当前工程使用 `.pro` 管理）
- Linux 图形环境（X11 / Wayland）

### 1.2 Ubuntu 参考安装

```bash
sudo apt update
sudo apt install -y \
  qtbase5-dev qtdeclarative5-dev qttools5-dev qttools5-dev-tools \
  libqt5serialport5-dev libqwt-qt5-dev build-essential
```

> 说明：不同发行版包名略有差异。若系统只提供 `libqwt-dev`，请按发行版实际包名调整。

---

## 2. 工程结构与职责边界

```text
ConfigStudio/
├── app/            # 全局上下文、模式枚举、应用级配置
├── ui/             # 主窗口与属性面板协调逻辑
├── canvas/         # 画布、组件基类、编辑行为（选中/拖拽/缩放）
├── components/     # 具体组件实现（plot/text/indicator/...）
├── model/          # 变量、页面、工程数据模型
├── runtime/        # 运行态窗口、绑定管理、仿真
├── datasrc/        # 串口等数据源适配
├── commands/       # 命令与撤销重做栈基础设施
├── utils/          # 辅助工具
├── virtualkey/     # 软键盘与触屏输入相关
├── resources/      # 资源文件（图标、qml 等）
└── docs/           # 开发说明
```

### 2.1 设计态与运行态

- **设计态（Design Mode）**：允许布局编辑、选中、拖拽、尺寸调整、属性修改。
- **运行态（Runtime Mode）**：默认锁定编辑行为，仅保留业务交互，降低误操作风险。

### 2.2 关键数据流（建议牢记）

1. 用户在画布中选中组件；
2. UI 层读取组件 `properties()` 并渲染属性表；
3. 属性变更回调组件 `setPropertyValue(key, value)`；
4. 绑定属性（如 `varId`）由运行层接管；
5. `DataBindingManager` 将变量更新分发到目标组件；
6. 组件更新显示并触发重绘。

---

## 3. 构建与运行

### 3.1 常规构建

```bash
qmake ConfigStudio.pro
make -j$(nproc)
```

### 3.2 启动应用

```bash
./ConfigStudio
```

### 3.3 属性诊断日志（推荐）

```bash
CONFIGSTUDIO_PROP_DIAG=1 ./ConfigStudio
```

用于定位：

- 属性点击进入分支是否正确；
- `setPropertyValue` 是否收到了预期 key/value；
- 绑定更新是否到达目标组件。

---

## 4. 新增组件开发清单（Checklist）

新增组件建议按以下步骤执行：

1. **定义类型标识**：实现 `type()` 并确保唯一；
2. **暴露属性**：在 `properties()` 中列出完整、可解释的键值；
3. **回写逻辑**：在 `setPropertyValue()` 做类型转换与容错；
4. **绑定接入**：明确哪些属性可绑定 `varId`；
5. **重绘反馈**：属性变化后触发 UI 可见更新；
6. **运行态约束**：遵循 edit-lock，避免运行态误编辑；
7. **自测补齐**：至少覆盖“属性回写 + 绑定分发 + 边界输入”。

---

## 5. 提交流程建议

```bash
git checkout -b feat/xxx
# 开发与自测
git add -A
git commit -m "feat: xxx"
```

建议提交说明包含：

- 变更动机（为什么做）；
- 影响范围（模块、组件、运行态行为）；
- 验证方式（命令 + 结果）；
- 已知限制（如环境缺依赖导致某测试未执行）。

---

## 6. CI/自动化建议（可选）

后续可增加：

- GitHub Actions（安装 Qt + Qwt 后执行模型层测试）；
- 提交前钩子（格式检查 + 最小单测）；
- 覆盖率报告（聚焦 model/runtime 核心逻辑）。
