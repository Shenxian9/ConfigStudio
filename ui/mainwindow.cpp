#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QSignalBlocker>
#include <QPointer>
#include <QLoggingCategory>
#include <QDateTime>
#include <QColor>
#include <QInputMethod>
#include <QSet>
#include <QFormLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QTableWidget>
#include <QTreeView>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QIntValidator>
#include <QMouseEvent>
#include <QScroller>
#include <algorithm>

Q_LOGGING_CATEGORY(propDiag, "configstudio.property")

namespace {
bool propertyDiagEnabled()
{
    return qEnvironmentVariableIsSet("CONFIGSTUDIO_PROP_DIAG");
}

void propDiagLog(const QString &msg)
{
    if (!propertyDiagEnabled()) return;
    qCInfo(propDiag).noquote() << QDateTime::currentDateTime().toString("hh:mm:ss.zzz") << msg;
}

constexpr int kPropertyValueTypeRole = Qt::UserRole + 1;

QString normalizedColorName(const QString &raw)
{
    const QString v = raw.trimmed().toLower();
    if (v == "gray" || v == "grey" || v == "red" || v == "green" || v == "blue")
        return v;

    const QColor c(v);
    if (!c.isValid())
        return "gray";

    if (c == QColor(128, 128, 128)) return "gray";
    if (c == QColor(255, 0, 0)) return "red";
    if (c == QColor(0, 255, 0)) return "green";
    if (c == QColor(0, 0, 255)) return "blue";

    return "gray";
}

QString normalizedTextColorName(const QString &raw)
{
    const QString v = raw.trimmed().toLower();
    if (v == "black" || v == "gray" || v == "grey" ||
        v == "red" || v == "green" || v == "blue" || v == "yellow")
        return v == "grey" ? "gray" : v;

    const QColor c(v);
    if (!c.isValid())
        return "black";

    if (c == QColor("black")) return "black";
    if (c == QColor("gray") || c == QColor("grey") || c == QColor(128, 128, 128)) return "gray";
    if (c == QColor("red") || c == QColor(255, 0, 0)) return "red";
    if (c == QColor("green") || c == QColor(0, 255, 0)) return "green";
    if (c == QColor("blue") || c == QColor(0, 0, 255)) return "blue";
    if (c == QColor("yellow") || c == QColor(255, 255, 0)) return "yellow";

    return "black";
}

QString nextColorName(const QString &current)
{
    const QString c = normalizedColorName(current);
    if (c == "gray") return "red";
    if (c == "red") return "green";
    if (c == "green") return "blue";
    return "gray";
}

QString nextTextColorName(const QString &current)
{
    const QString c = current.trimmed().toLower();
    if (c == "black") return "red";
    if (c == "red") return "green";
    if (c == "green") return "blue";
    if (c == "blue") return "yellow";
    if (c == "yellow") return "gray";
    return "black";
}

bool isColorPropertyKey(const QString &key)
{
    return key == "onColor" || key == "textColor" || key == "textcolor" || key == "color" || key == "offColor";
}

bool isLikelyBoolPropertyKey(const QString &key)
{
    const QString k = key.trimmed().toLower();
    static const QSet<QString> boolKeys = {
        "on", "blink", "checked", "enabled", "visible", "readonly", "blackbg"
    };
    return boolKeys.contains(k);
}

bool parseBoolText(const QString &text, bool *ok)
{
    const QString s = text.trimmed().toLower();
    if (s == "true" || s == "1" || s == "on") {
        if (ok) *ok = true;
        return true;
    }
    if (s == "false" || s == "0" || s == "off") {
        if (ok) *ok = true;
        return false;
    }
    if (ok) *ok = false;
    return false;
}
}

void setLabelIcon(QLabel* label, const QString& path)
{
    QPixmap pix(path);
    if (pix.isNull()) return;

    pix = pix.scaled(label->size(),
                     Qt::KeepAspectRatio,
                     Qt::SmoothTransformation);
    label->setPixmap(pix);
    label->setAlignment(Qt::AlignCenter);
}


MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    qApp->installEventFilter(this);
    propDiagLog(QString("MainWindow init, platform=%1").arg(QGuiApplication::platformName()));
    QScreen *screen = QApplication::primaryScreen();
    QSize size = screen->size();

    qDebug() << QGuiApplication::platformName();
    qDebug() << qApp->inputMethod()->isVisible();

    resize(size.width(),size.height());//720x1280
    move(0, 0);






    PaletteBinder *binder = new PaletteBinder(this);
    binder->bind(ui->labelOfPlot,  "plot");
    ui->labelOfPlot->setIcon(":/icons/plot.png");
    binder->bind(ui->labelOfDial,  "dial");
    ui->labelOfDial->setIcon(":/icons/dial.png");
    binder->bind(ui->labelOfThermo,  "thermo");
    ui->labelOfThermo->setIcon(":/icons/bar.png");
    binder->bind(ui->labelOfSlider,  "slider");
    ui->labelOfSlider->setIcon(":/icons/slider.png");
    binder->bind(ui->labelOfText,  "text");
    ui->labelOfText->setIcon(":/icons/text.png");
    binder->bind(ui->labelOfHistogram,  "histogram");
    ui->labelOfHistogram->setIcon(":/icons/histogram.png");
    binder->bind(ui->labelOfSwitch,  "switch");
    ui->labelOfSwitch->setIcon(":/icons/switch.png");
    binder->bind(ui->labelOfWheel,  "wheel");
    ui->labelOfWheel->setIcon(":/icons/wheel.png");
    binder->bind(ui->labelOfIndicator,  "indicator");
    ui->labelOfIndicator->setIcon(":/icons/indicator.png");
    binder->bind(ui->labelOfNumeric,  "numeric");
    ui->labelOfNumeric->setIcon(":/icons/numeric.png");
    setupIconButton(ui->buttonOfFullscreen, ":/icons/fullscreen.png");
    setupIconButton(ui->deleteButton, ":/icons/delete.png");
    setupIconButton(ui->pushOfDatasrc, ":/icons/datasource.png");
    setupIconButton(ui->pushOfDesign, ":/icons/designmode.png");
    refreshActionButtonIcons();
    //ui->pushOfL_D->setText("darkmode");
    applyCanvasTheme(false);
    QTimer::singleShot(0, this, [this]() { enforceCanvasFrameRatio(); });

    connect(ui->canvasView, &CanvasView::itemSelected,
            this, &MainWindow::onItemSelected);

    connect(ui->propertyTable, &QTableWidget::cellChanged,
            this, &MainWindow::onPropertyChanged);

    // 属性表本体不走输入法，文本输入统一通过弹窗处理（Wayland/IME 更稳定）
    ui->propertyTable->setAttribute(Qt::WA_InputMethodEnabled, false);

    connect(ui->propertyTable, &QTableWidget::cellClicked,
            this, &MainWindow::editPropertyCell);

    connect(ui->deleteButton, &QPushButton::clicked,
            this, &MainWindow::on_deleteButton_clicked);

    connect(ui->canvasView, &CanvasView::emptyAreaClicked,
            this, &MainWindow::clearProperties);

    connect(ui->pushButton_6, &QPushButton::clicked, this, [this]() {
        if (!ui) return;

        enforceCanvasFrameRatio();

        const auto dump = [](const char *name, QWidget *w) {
            if (!w) {
                qDebug() << "[layout-debug]" << name << "is null";
                return;
            }
            qDebug() << "[layout-debug]" << name
                     << "w=" << w->width()
                     << "h=" << w->height();
        };

        dump("canvasView", ui->canvasView);
        dump("frame", ui->frame);
        dump("propertyTable", ui->propertyTable);
        dump("widget_3", ui->widget_3);
    });

    // ① 数据模型
    m_variableModel = new VariableModel(this);


    // ② 绑定管理器（核心）
    m_bindingMgr = new DataBindingManager(m_variableModel, this);


    // ③ ⭐ 把核心注入 UI 里的 CanvasView
    ui->canvasView->setBindingManager(m_bindingMgr);



    m_canvas = ui->canvasView;

    ui->variableView->setModel(m_variableModel);
    ui->variableView->setModel(m_variableModel);
    ui->variableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->variableView->horizontalHeader()->setStretchLastSection(true);
    ui->variableView->horizontalHeader()->setSectionResizeMode(VariableModel::ColValue, QHeaderView::Interactive);
    ui->variableView->setColumnWidth(VariableModel::ColValue, 240);

    // ⭐ 2. 垂直表头隐藏（更干净）
    ui->variableView->verticalHeader()->setVisible(false);

    // ⭐ 3. 行高自适应字体
    ui->variableView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);



    // ⭐ 5. 选中整行
    ui->variableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->variableView->setSelectionMode(QAbstractItemView::SingleSelection);

    // ⭐ 6. 禁止编辑时出现虚线框
    ui->variableView->setFocusPolicy(Qt::NoFocus);
    QFont f;
    f.setPointSize(14);     // ⭐ 13~15 是这个分辨率的最佳区间
    ui->variableView->setFont(f);
    ui->variableView->verticalHeader()->setDefaultSectionSize(40);
    ui->variableView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    ui->variableView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->variableView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    QScroller::grabGesture(ui->variableView->viewport(), QScroller::LeftMouseButtonGesture);
    connect(ui->variableView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &MainWindow::updateVariableActionButtons);
    connect(ui->variableView->selectionModel(), &QItemSelectionModel::currentChanged,
            this, [this](const QModelIndex &, const QModelIndex &) { updateVariableActionButtons(); });
    updateVariableViewColumns();




    qDebug() << "Main model =" << m_variableModel;
    // 添加测试变量（关键是 strategy）
    Variable v1;
    v1.id = "1";
    v1.name = "Temp";
    v1.deviceId = "PLC1";
    v1.type = "float";
    v1.unit = "℃";
    v1.strategy = DataStrategy::Sine;

    Variable v2;
    v2.id = "2";
    v2.name = "Pressure";
    v2.deviceId = "PLC2";
    v2.type = "float";
    v2.unit = "Pa";
    v2.strategy = DataStrategy::Random;

    Variable v3;
    v3.id = "3";
    v3.name = "Age";
    v3.deviceId = "PLC3";
    v3.type = "float";
    v3.unit = "year";
    v3.strategy = DataStrategy::Square;

    Variable v4;
    v4.id = "Variable";
    v4.name = "Variable";
    v4.deviceId = "TEST";
    v4.type = "float";
    v4.unit = "";
    v4.value = 50.0;
    v4.strategy = DataStrategy::None;

    m_variableModel->addVariable(v1);
    m_variableModel->addVariable(v2);
    m_variableModel->addVariable(v3);
    m_variableModel->addVariable(v4);

    setupDataWorkspace();

    // 启动仿真
    m_runtimeSimulator = new RuntimeSimulator(m_variableModel, this);
    m_runtimeSimulator->start(300);


}

void MainWindow::setupDataWorkspace()
{
    m_serialDataSource = new SerialDataSource(this); // 保留配置承载
    m_modbusDataSource = new ModbusRtuDataSource(this);
    m_modbusDataSource->setVariableModel(m_variableModel);
    m_modbusDataSource->setConfig(m_serialDataSource->config());
    m_bindingMgr->setWriteBackend(m_modbusDataSource);
    m_dataSourceTreeModel = new QStandardItemModel(this);

    m_dataSourceTreeModel->setHorizontalHeaderLabels({"Data Sources"});
    ui->treeView->setModel(m_dataSourceTreeModel);
    ui->treeView->header()->setStretchLastSection(true);
    QFont dataSourceTreeFont = ui->treeView->font();
    dataSourceTreeFont.setPointSize(qMax(14, dataSourceTreeFont.pointSize() + 4));
    ui->treeView->setFont(dataSourceTreeFont);
    ui->treeView->setIconSize(QSize(28, 28));
    ui->treeView->setIndentation(28);
    ui->treeView->header()->setMinimumSectionSize(260);
    ui->treeView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    ui->treeView->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    ui->treeView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->treeView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    QScroller::grabGesture(ui->treeView->viewport(), QScroller::LeftMouseButtonGesture);
    ui->treeView->setStyleSheet(
        "QTreeView::item { min-height: 46px; padding: 6px 2px; }"
        "QTreeView::branch { min-width: 30px; min-height: 46px; }");
    ui->treeView->viewport()->installEventFilter(this);
    m_modbusConfigs.clear();
    m_modbusConfigs.push_back(m_serialDataSource->config());

    connect(ui->treeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, [this]() {
        updateDataSourceActionButtons();
    });

    connect(m_modbusDataSource, &ModbusRtuDataSource::statusChanged, this, [this](bool opened) {
        ui->pushButton_8->setEnabled(!opened);
        ui->pushButton_9->setEnabled(opened);
        refreshDataSourceTreeDeferred();
    });

    connect(m_modbusDataSource, &ModbusRtuDataSource::pollingStateChanged, this, [this](bool running) {
        Q_UNUSED(running);
        refreshDataSourceTreeDeferred();
    });

    connect(m_modbusDataSource, &ModbusRtuDataSource::errorOccurred, this, [this](const QString &err) {
        m_lastCommStatus = err;
        showErrorNotice(tr("Modbus RTU Data Source"), err);
        refreshDataSourceTreeDeferred();
    });
    connect(m_modbusDataSource, &ModbusRtuDataSource::variableReadSucceeded, this, [this](const QString &varId, const QVariant &value) {
        m_lastCommStatus = QString("OK %1=%2").arg(varId, value.toString());
        refreshDataSourceTreeDeferred();
    });
    connect(m_modbusDataSource, &ModbusRtuDataSource::variableWriteSucceeded, this, [this](const QString &varId) {
        m_lastCommStatus = QString("Write OK: %1").arg(varId);
        refreshDataSourceTreeDeferred();
    });
    connect(m_modbusDataSource, &ModbusRtuDataSource::variableWriteFailed, this, [this](const QString &varId, const QString &reason) {
        m_lastCommStatus = QString("Write Fail %1: %2").arg(varId, reason);
        showErrorNotice(tr("Modbus Write Failed"), m_lastCommStatus);
        refreshDataSourceTreeDeferred();
    });

    connect(ui->pushButton_2, &QPushButton::clicked, this, &MainWindow::showSerialConfigDialog);
    connect(ui->pushButton_8, &QPushButton::clicked, this, [this]() {
        if (!m_modbusDataSource->open())
            return;
        if (m_dataSourceModeCombo && m_dataSourceModeCombo->text() == "Modbus RTU")
            m_modbusDataSource->startPolling();
        refreshDataSourceTreeDeferred();
    });
    connect(ui->pushButton_9, &QPushButton::clicked, this, [this]() {
        m_modbusDataSource->close();
        refreshDataSourceTreeDeferred();
    });
    connect(ui->pushButton_7, &QPushButton::clicked, this, [this]() {
        const int row = selectedDataSourceRow();
        if (row < 0 || row >= m_modbusConfigs.size())
            return;

        m_modbusDataSource->close();
        m_modbusConfigs.removeAt(row);
        if (m_modbusConfigs.isEmpty()) {
            const SerialPortConfig emptyCfg;
            m_serialDataSource->setConfig(emptyCfg);
            m_modbusDataSource->setConfig(emptyCfg);
            if (ui->treeView && ui->treeView->selectionModel()) {
                ui->treeView->selectionModel()->setCurrentIndex(QModelIndex(), QItemSelectionModel::Clear);
                ui->treeView->selectionModel()->clearSelection();
            }
        } else {
            const int nextRow = qBound(0, row, m_modbusConfigs.size() - 1);
            const SerialPortConfig cfg = m_modbusConfigs.at(nextRow);
            m_serialDataSource->setConfig(cfg);
            m_modbusDataSource->setConfig(cfg);
            if (ui->treeView && ui->treeView->selectionModel()) {
                const QModelIndex idx = m_dataSourceTreeModel->index(nextRow, 0);
                ui->treeView->selectionModel()->setCurrentIndex(idx, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
            }
        }
        refreshDataSourceTreeDeferred();
    });

    ui->pushButton_2->setObjectName("dataSourceConfigButton");
    ui->pushButton_7->setObjectName("dataSourceRemoveButton");
    ui->pushButton_8->setObjectName("dataSourceOpenButton");
    ui->pushButton_9->setObjectName("dataSourceCloseButton");
    ui->pushButton_10->setObjectName("addVariableButton");
    ui->pushButton_11->setObjectName("deleteVariableButton");
    ui->pushButton_12->setObjectName("editVariableButton");
    ui->dataSourceModeCombo->setObjectName("dataSourceModeCombo");
    m_dataSourceModeCombo = ui->dataSourceModeCombo;
    connect(m_dataSourceModeCombo, &QPushButton::clicked, this, [this]() {
        if (!m_dataSourceModeCombo)
            return;
        m_dataSourceModeCombo->setText(m_dataSourceModeCombo->text() == "Simulator" ? "Modbus RTU" : "Simulator");
        applyDataSourceMode();
    });

    connect(ui->pushButton_10, &QPushButton::clicked, this, &MainWindow::showAddVariableDialog);
    connect(ui->pushButton_11, &QPushButton::clicked, this, &MainWindow::deleteSelectedVariable);
    connect(ui->pushButton_12, &QPushButton::clicked, this, &MainWindow::showEditVariableDialog);

    ui->pushButton_8->setEnabled(true);
    ui->pushButton_9->setEnabled(false);
    updateVariableActionButtons();
    updateDataSourceActionButtons();

    setupDataWorkspacePanels();
    applyDataSourceMode();
    refreshDataSourceTree();
}

void MainWindow::setupDataWorkspacePanels()
{
    if (!m_serialConfigPanel) {
        m_serialConfigPanel = new QFrame(this);
        m_serialConfigPanel->setObjectName("serialConfigPanel");
        m_serialConfigPanel->setStyleSheet("#serialConfigPanel { background: #f7f7f7; border: 1px solid #888; border-radius: 6px; }");
        m_serialConfigPanel->hide();

        auto *layout = new QVBoxLayout(m_serialConfigPanel);
        layout->setContentsMargins(12, 12, 12, 12);
        layout->setSpacing(8);

        auto *title = new QLabel("Modbus RTU Connection Config", m_serialConfigPanel);
        QFont titleFont = title->font();
        titleFont.setBold(true);
        titleFont.setPointSize(titleFont.pointSize() + 1);
        title->setFont(titleFont);
        layout->addWidget(title);

        auto *serialGroup = new QGroupBox("Serial Link Parameters", m_serialConfigPanel);
        auto *serialForm = new QFormLayout(serialGroup);
        serialForm->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
        serialForm->setSpacing(10);

        m_serialDeviceEdit = new QLineEdit(m_serialConfigPanel);
        m_serialDeviceEdit->setObjectName("serialDeviceEdit");
        m_serialDeviceEdit->setPlaceholderText("default");
        m_serialPortEdit = new QLineEdit(m_serialConfigPanel);
        m_serialPortEdit->setObjectName("serialPortEdit");
        m_serialPortEdit->setPlaceholderText("/dev/ttyS2 or COM3");
        m_serialBaudCombo = new OptionCycleButton(m_serialConfigPanel);
        m_serialBaudCombo->setObjectName("serialBaudCombo");
        m_serialBaudCombo->addItems({"1200", "2400", "4800", "9600", "19200", "38400", "57600", "115200"});
        m_dataBitsCombo = new OptionCycleButton(m_serialConfigPanel);
        m_dataBitsCombo->setObjectName("serialDataBitsCombo");
        m_dataBitsCombo->addItems({"5", "6", "7", "8"});
        m_parityCombo = new OptionCycleButton(m_serialConfigPanel);
        m_parityCombo->setObjectName("serialParityCombo");
        m_parityCombo->addItems({"None", "Even", "Odd"});
        m_stopBitsCombo = new OptionCycleButton(m_serialConfigPanel);
        m_stopBitsCombo->setObjectName("serialStopBitsCombo");
        m_stopBitsCombo->addItems({"1", "2"});
        m_serialBaudCombo->setMinimumSize(260, 46);
        m_dataBitsCombo->setMinimumSize(260, 46);
        m_parityCombo->setMinimumSize(260, 46);
        m_stopBitsCombo->setMinimumSize(260, 46);
        registerTouchInput(m_serialDeviceEdit);
        registerTouchInput(m_serialPortEdit);

        serialForm->addRow("Device", m_serialDeviceEdit);
        serialForm->addRow("Port", m_serialPortEdit);
        serialForm->addRow("BaudRate", m_serialBaudCombo);
        serialForm->addRow("DataBits", m_dataBitsCombo);
        serialForm->addRow("Parity", m_parityCombo);
        serialForm->addRow("StopBits", m_stopBitsCombo);
        auto *groupRow = new QHBoxLayout();
        groupRow->setSpacing(18);
        groupRow->addWidget(serialGroup, 1);

        auto *modbusGroup = new QGroupBox("Modbus Parameters", m_serialConfigPanel);
        auto *modbusForm = new QFormLayout(modbusGroup);
        modbusForm->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
        modbusForm->setSpacing(10);

        m_slaveIdSpin = new QSpinBox(m_serialConfigPanel);
        m_slaveIdSpin->setObjectName("serialSlaveIdSpin");
        m_slaveIdSpin->setRange(1, 247);
        m_timeoutSpin = new QSpinBox(m_serialConfigPanel);
        m_timeoutSpin->setObjectName("serialTimeoutSpin");
        m_timeoutSpin->setRange(50, 60000);
        m_timeoutSpin->setSingleStep(50);
        m_retrySpin = new QSpinBox(m_serialConfigPanel);
        m_retrySpin->setObjectName("serialRetrySpin");
        m_retrySpin->setRange(0, 10);
        m_pollIntervalSpin = new QSpinBox(m_serialConfigPanel);
        m_pollIntervalSpin->setObjectName("serialPollIntervalSpin");
        m_pollIntervalSpin->setRange(50, 60000);
        m_pollIntervalSpin->setSingleStep(50);
        m_functionCodeCombo = new OptionCycleButton(m_serialConfigPanel);
        m_functionCodeCombo->setObjectName("serialFunctionCodeCombo");
        m_functionCodeCombo->addItems({"03 - Read Holding Registers", "04 - Read Input Registers", "06 - Write Single Register", "10 - Write Multiple Registers"});
        m_functionCodeCombo->setMinimumSize(260, 46);
        registerTouchInput(m_slaveIdSpin);
        registerTouchInput(m_timeoutSpin);
        registerTouchInput(m_retrySpin);
        registerTouchInput(m_pollIntervalSpin);

        modbusForm->addRow("Slave ID", m_slaveIdSpin);
        modbusForm->addRow("Timeout (ms)", m_timeoutSpin);
        modbusForm->addRow("Retry Count", m_retrySpin);
        modbusForm->addRow("Poll Interval (ms)", m_pollIntervalSpin);
        modbusForm->addRow("Default Function Code", m_functionCodeCombo);
        groupRow->addWidget(modbusGroup, 1);
        layout->addLayout(groupRow);

        auto *hint = new QLabel("Phase 1 only consolidates the device-level entry. Register-level mapping will move into the variable model in phase 2. Default Function Code is temporarily retained and not used for real protocol scheduling.", m_serialConfigPanel);
        hint->setWordWrap(true);
        hint->setStyleSheet("color:#666;");
        layout->addWidget(hint);

        auto *buttons = new QHBoxLayout();
        auto *testBtn = new QPushButton("Test Connection", m_serialConfigPanel);
        testBtn->setEnabled(false);
        auto *okBtn = new QPushButton("Apply", m_serialConfigPanel);
        auto *cancelBtn = new QPushButton("Cancel", m_serialConfigPanel);
        for (QPushButton *btn : {testBtn, okBtn, cancelBtn}) {
            btn->setMinimumSize(130, 50);
            btn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        }
        buttons->setSpacing(12);
        buttons->addStretch();
        buttons->addWidget(testBtn);
        buttons->addWidget(okBtn);
        buttons->addWidget(cancelBtn);
        layout->addLayout(buttons);

        connect(okBtn, &QPushButton::clicked, this, &MainWindow::applySerialConfigFromPanel);
        connect(cancelBtn, &QPushButton::clicked, this, &MainWindow::hideDataWorkspacePanels);
    }

    if (!m_variableEditorPanel) {
        m_variableEditorPanel = new QFrame(this);
        m_variableEditorPanel->setObjectName("variableEditorPanel");
        m_variableEditorPanel->setStyleSheet("#variableEditorPanel { background: #f7f7f7; border: 1px solid #888; border-radius: 6px; }");
        m_variableEditorPanel->hide();

        auto *layout = new QVBoxLayout(m_variableEditorPanel);
        layout->setContentsMargins(12, 12, 12, 12);
        layout->setSpacing(8);

        auto *title = new QLabel("Variable Config / Modbus Mapping", m_variableEditorPanel);
        QFont titleFont = title->font();
        titleFont.setBold(true);
        titleFont.setPointSize(titleFont.pointSize() + 1);
        title->setFont(titleFont);
        layout->addWidget(title);

        auto *leftForm = new QFormLayout();
        leftForm->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
        leftForm->setSpacing(10);
        auto *rightForm = new QFormLayout();
        rightForm->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
        rightForm->setSpacing(10);

        m_variableIdEdit = new QLineEdit(m_variableEditorPanel);
        m_variableIdEdit->setObjectName("variableIdEdit");
        m_variableNameEdit = new QLineEdit(m_variableEditorPanel);
        m_variableNameEdit->setObjectName("variableNameEdit");
        m_variableDeviceEdit = new QLineEdit(m_variableEditorPanel);
        m_variableDeviceEdit->setObjectName("variableDeviceEdit");
        m_variableTypeCombo = new OptionCycleButton(m_variableEditorPanel);
        m_variableTypeCombo->setObjectName("variableTypeCombo");
        m_variableTypeCombo->addItems({"bool", "int16", "uint16", "int32", "uint32", "float32"});
        m_variableAreaCombo = new OptionCycleButton(m_variableEditorPanel);
        m_variableAreaCombo->setObjectName("variableAreaCombo");
        m_variableAreaCombo->addItems({"Coil", "DiscreteInput", "InputRegister", "HoldingRegister"});
        m_variableAddressSpin = new QSpinBox(m_variableEditorPanel);
        m_variableAddressSpin->setObjectName("variableAddressSpin");
        m_variableAddressSpin->setRange(0, 1000000);
        m_variableCountSpin = new QSpinBox(m_variableEditorPanel);
        m_variableCountSpin->setObjectName("variableCountSpin");
        m_variableCountSpin->setRange(1, 125);
        m_variableBitOffsetSpin = new QSpinBox(m_variableEditorPanel);
        m_variableBitOffsetSpin->setObjectName("variableBitOffsetSpin");
        m_variableBitOffsetSpin->setRange(0, 63);
        m_variableUnitEdit = new QLineEdit(m_variableEditorPanel);
        m_variableUnitEdit->setObjectName("variableUnitEdit");
        m_variableScaleSpin = new QDoubleSpinBox(m_variableEditorPanel);
        m_variableScaleSpin->setObjectName("variableScaleSpin");
        m_variableScaleSpin->setRange(0.000001, 1000000.0);
        m_variableScaleSpin->setDecimals(6);
        m_variableScaleSpin->setValue(1.0);
        m_variableReadOnlyCheck = new QCheckBox("Read Only", m_variableEditorPanel);
        m_variableReadOnlyCheck->setObjectName("variableReadOnlyCheck");
        m_variableReadOnlyCheck->setChecked(true);
        m_variableReadOnlyCheck->setMinimumHeight(46);
        m_variableReadOnlyCheck->setStyleSheet("QCheckBox::indicator { width: 32px; height: 32px; }");
        m_variableEndianCombo = new OptionCycleButton(m_variableEditorPanel);
        m_variableEndianCombo->setObjectName("variableEndianCombo");
        m_variableEndianCombo->addItems({"BigEndian", "BigEndianWordSwap", "LittleEndian", "LittleEndianByteSwap"});
        m_variableTypeCombo->setMinimumSize(260, 46);
        m_variableAreaCombo->setMinimumSize(260, 46);
        m_variableEndianCombo->setMinimumSize(260, 46);
        registerTouchInput(m_variableIdEdit);
        registerTouchInput(m_variableNameEdit);
        registerTouchInput(m_variableDeviceEdit);
        registerTouchInput(m_variableAddressSpin);
        registerTouchInput(m_variableCountSpin);
        registerTouchInput(m_variableBitOffsetSpin);
        registerTouchInput(m_variableUnitEdit);
        registerTouchInput(m_variableScaleSpin);

        leftForm->addRow("Variable ID", m_variableIdEdit);
        leftForm->addRow("Name", m_variableNameEdit);
        leftForm->addRow("Device", m_variableDeviceEdit);
        leftForm->addRow("Type", m_variableTypeCombo);
        leftForm->addRow("Area", m_variableAreaCombo);
        leftForm->addRow("Address", m_variableAddressSpin);

        rightForm->addRow("Count", m_variableCountSpin);
        rightForm->addRow("Bit Offset", m_variableBitOffsetSpin);
        rightForm->addRow("Unit", m_variableUnitEdit);
        rightForm->addRow("Scale", m_variableScaleSpin);
        rightForm->addRow("ReadOnly", m_variableReadOnlyCheck);
        rightForm->addRow("Endianness", m_variableEndianCombo);

        auto *formColumns = new QHBoxLayout();
        formColumns->setSpacing(18);
        formColumns->addLayout(leftForm, 1);
        formColumns->addLayout(rightForm, 1);
        layout->addLayout(formColumns);

        auto *buttons = new QHBoxLayout();
        auto *applyBtn = new QPushButton("Apply", m_variableEditorPanel);
        applyBtn->setObjectName("variableApplyButton");
        auto *cancelBtn = new QPushButton("Cancel", m_variableEditorPanel);
        cancelBtn->setObjectName("variableCancelButton");
        for (QPushButton *btn : {applyBtn, cancelBtn}) {
            btn->setMinimumSize(130, 50);
            btn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        }
        buttons->setSpacing(12);
        buttons->addStretch();
        buttons->addWidget(applyBtn);
        buttons->addWidget(cancelBtn);
        layout->addLayout(buttons);

        connect(applyBtn, &QPushButton::clicked, this, &MainWindow::applyVariableFromPanel);
        connect(cancelBtn, &QPushButton::clicked, this, &MainWindow::hideDataWorkspacePanels);
    }

}

void MainWindow::ensureErrorNoticePanel()
{
    if (m_errorNoticePanel)
        return;

    m_errorNoticePanel = new QFrame(this);
    m_errorNoticePanel->setFrameShape(QFrame::StyledPanel);
    m_errorNoticePanel->setStyleSheet("QFrame { background: white; border: 2px solid #d97a7a; border-radius: 8px; }");
    m_errorNoticePanel->hide();

    auto *layout = new QVBoxLayout(m_errorNoticePanel);
    m_errorNoticeTitleLabel = new QLabel(m_errorNoticePanel);
    QFont titleFont = m_errorNoticeTitleLabel->font();
    titleFont.setBold(true);
    titleFont.setPointSize(14);
    m_errorNoticeTitleLabel->setFont(titleFont);
    layout->addWidget(m_errorNoticeTitleLabel);

    m_errorNoticeMessageLabel = new QLabel(m_errorNoticePanel);
    m_errorNoticeMessageLabel->setWordWrap(true);
    QFont msgFont = m_errorNoticeMessageLabel->font();
    msgFont.setPointSize(12);
    m_errorNoticeMessageLabel->setFont(msgFont);
    layout->addWidget(m_errorNoticeMessageLabel);

    auto *okBtn = new QPushButton("OK", m_errorNoticePanel);
    okBtn->setMinimumHeight(38);
    okBtn->setFocusPolicy(Qt::NoFocus);
    layout->addWidget(okBtn);
    connect(okBtn, &QPushButton::clicked, m_errorNoticePanel, &QWidget::hide);
}

void MainWindow::showErrorNotice(const QString &title, const QString &message)
{
    ensureErrorNoticePanel();
    if (!m_errorNoticePanel || !m_errorNoticeTitleLabel || !m_errorNoticeMessageLabel)
        return;

    m_errorNoticeTitleLabel->setText(title);
    m_errorNoticeMessageLabel->setText(message);

    const int margin = 14;
    const int panelW = qMin(width() - margin * 2, 560);
    const int panelH = 190;
    m_errorNoticePanel->setGeometry((width() - panelW) / 2,
                                    qMax(10, (height() - panelH) / 2 - height() / 8),
                                    panelW, panelH);
    m_errorNoticePanel->raise();
    m_errorNoticePanel->show();
}

void MainWindow::applyDataSourceMode()
{
    const QString mode = m_dataSourceModeCombo ? m_dataSourceModeCombo->text() : QString("Simulator");
    if (mode == "Modbus RTU") {
        if (m_modbusDataSource)
            m_modbusDataSource->setWriteEnabled(true);
        if (m_runtimeSimulator)
            m_runtimeSimulator->stop();
        if (m_modbusDataSource && m_modbusDataSource->isOpen())
            m_modbusDataSource->startPolling();
    } else {
        if (m_modbusDataSource)
            m_modbusDataSource->setWriteEnabled(false);
        if (m_modbusDataSource)
            m_modbusDataSource->stopPolling();
        if (m_runtimeSimulator && !m_runtimeSimulator->isRunning())
            m_runtimeSimulator->start(300);
    }
    updateVariableViewColumns();
    refreshDataSourceTreeDeferred();
}

void MainWindow::updateVariableViewColumns()
{
    if (!ui->variableView || !ui->variableView->model() || !m_variableModel)
        return;

    const QSet<int> baseVisibleColumns = {
        VariableModel::ColId,
        VariableModel::ColName,
        VariableModel::ColDevice,
        VariableModel::ColType,
        VariableModel::ColAddress,
        VariableModel::ColCount,
        VariableModel::ColValue,
        VariableModel::ColBitOffset,
        VariableModel::ColScale,
        VariableModel::ColReadOnly
    };
    const bool showStrategy = m_dataSourceModeCombo && m_dataSourceModeCombo->text() == "Simulator";

    for (int col = 0; col < m_variableModel->columnCount({}); ++col) {
        const bool visible = baseVisibleColumns.contains(col) || (showStrategy && col == VariableModel::ColStrategy);
        ui->variableView->setColumnHidden(col, !visible);
    }
    ui->variableView->horizontalHeader()->setSectionResizeMode(VariableModel::ColValue, QHeaderView::Interactive);
    ui->variableView->setColumnWidth(VariableModel::ColValue, qMax(240, ui->variableView->columnWidth(VariableModel::ColValue)));
}

void MainWindow::registerTouchInput(QWidget *editor)
{
    if (!editor)
        return;
    editor->setMinimumHeight(46);
    editor->setMinimumWidth(260);
    m_touchInputs.insert(editor);
    editor->installEventFilter(this);
    m_touchInputTargets.insert(editor, editor);
    const auto children = editor->findChildren<QWidget*>();
    for (QWidget *child : children) {
        child->installEventFilter(this);
        m_touchInputTargets.insert(child, editor);
    }

    if (auto *line = qobject_cast<QLineEdit*>(editor))
        line->setReadOnly(true);
    if (auto *spin = qobject_cast<QSpinBox*>(editor))
        spin->setButtonSymbols(QAbstractSpinBox::NoButtons);
    if (auto *dspin = qobject_cast<QDoubleSpinBox*>(editor))
        dspin->setButtonSymbols(QAbstractSpinBox::NoButtons);
}

void MainWindow::showTouchInputPopup(const QString &title, const QString &value, const std::function<void (const QString &)> &apply)
{
    if (!m_touchInputPanel) {
        auto *panel = new QFrame(this);
        panel->setFrameShape(QFrame::StyledPanel);
        panel->setStyleSheet("QFrame { background: white; border: 2px solid #7aa7d9; border-radius: 8px; }");
        panel->hide();

        auto *layout = new QVBoxLayout(panel);
        m_touchInputEdit = new QLineEdit(panel);
        m_touchInputEdit->setAttribute(Qt::WA_InputMethodEnabled, true);
        QFont inputFont = m_touchInputEdit->font();
        inputFont.setPointSize(18);
        m_touchInputEdit->setFont(inputFont);
        m_touchInputEdit->setMinimumHeight(52);
        layout->addWidget(m_touchInputEdit);

        auto *okBtn = new QPushButton("Apply", panel);
        auto *cancelBtn = new QPushButton("Cancel", panel);
        okBtn->setMinimumHeight(40);
        cancelBtn->setMinimumHeight(40);
        layout->addWidget(okBtn);
        layout->addWidget(cancelBtn);

        connect(okBtn, &QPushButton::clicked, this, [this]() {
            if (m_touchInputApply && m_touchInputEdit)
                m_touchInputApply(m_touchInputEdit->text());
            if (m_touchInputPanel)
                m_touchInputPanel->hide();
            QTimer::singleShot(0, []() {
                QInputMethod *inputMethod = QGuiApplication::inputMethod();
                if (inputMethod) {
                    inputMethod->hide();
                    inputMethod->reset();
                }
            });
        });
        connect(cancelBtn, &QPushButton::clicked, this, [this]() {
            if (m_touchInputPanel)
                m_touchInputPanel->hide();
            QTimer::singleShot(0, []() {
                QInputMethod *inputMethod = QGuiApplication::inputMethod();
                if (inputMethod) {
                    inputMethod->hide();
                    inputMethod->reset();
                }
            });
        });
        m_touchInputPanel = panel;
    }

    if (!m_touchInputPanel || !m_touchInputEdit)
        return;

    Q_UNUSED(title);
    m_touchInputApply = apply;
    m_touchInputEdit->setText(value);

    const int panelW = qMax(320, width() / 2);
    const int panelH = 220;
    const int panelX = (width() - panelW) / 2;
    const int panelY = qMax(16, (height() - panelH) / 2 - height() / 6);
    m_touchInputPanel->setGeometry(panelX, panelY, panelW, panelH);
    m_touchInputPanel->show();
    m_touchInputPanel->raise();
    QTimer::singleShot(0, this, [this]() {
        if (m_touchInputEdit)
            m_touchInputEdit->setFocus(Qt::OtherFocusReason);
        QInputMethod *inputMethod = QGuiApplication::inputMethod();
        if (inputMethod)
            inputMethod->show();
    });
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress && ui->treeView && ui->treeView->selectionModel()) {
        QWidget *watchedWidget = qobject_cast<QWidget*>(watched);
        const bool inThisWindow = watchedWidget && (watchedWidget == this || this->isAncestorOf(watchedWidget));
        const bool inDataSourceTree = watchedWidget
                                      && (watchedWidget == ui->treeView
                                          || watchedWidget == ui->treeView->viewport()
                                          || ui->treeView->isAncestorOf(watchedWidget));
        const bool onDataSourceActions = watchedWidget
                                         && (watchedWidget == ui->pushButton_2
                                             || watchedWidget == ui->pushButton_7
                                             || watchedWidget == ui->pushButton_8
                                             || watchedWidget == ui->pushButton_9
                                             || watchedWidget == ui->dataSourceModeCombo
                                             || (ui->pushButton_2 && ui->pushButton_2->isAncestorOf(watchedWidget))
                                             || (ui->pushButton_7 && ui->pushButton_7->isAncestorOf(watchedWidget))
                                             || (ui->pushButton_8 && ui->pushButton_8->isAncestorOf(watchedWidget))
                                             || (ui->pushButton_9 && ui->pushButton_9->isAncestorOf(watchedWidget))
                                             || (ui->dataSourceModeCombo && ui->dataSourceModeCombo->isAncestorOf(watchedWidget)));
        if (inThisWindow && !inDataSourceTree && !onDataSourceActions && selectedDataSourceRow() >= 0) {
            ui->treeView->selectionModel()->setCurrentIndex(QModelIndex(), QItemSelectionModel::Clear);
            ui->treeView->selectionModel()->clearSelection();
            updateDataSourceActionButtons();
        }
    }

    if (ui->treeView && watched == ui->treeView->viewport() && event->type() == QEvent::MouseButtonPress) {
        const auto *mouseEvent = static_cast<QMouseEvent*>(event);
        const QModelIndex clickedIndex = ui->treeView->indexAt(mouseEvent->pos());
        if (!clickedIndex.isValid() && ui->treeView->selectionModel()) {
            ui->treeView->selectionModel()->setCurrentIndex(QModelIndex(), QItemSelectionModel::Clear);
            ui->treeView->selectionModel()->clearSelection();
            updateDataSourceActionButtons();
            return true;
        }
    }

    QWidget *watchedWidget = qobject_cast<QWidget*>(watched);
    QWidget *w = nullptr;
    if (watchedWidget && m_touchInputs.contains(watchedWidget))
        w = watchedWidget;
    else
        w = m_touchInputTargets.value(watched, nullptr);
    if (!w)
        return QWidget::eventFilter(watched, event);

    if (event->type() != QEvent::MouseButtonPress)
        return QWidget::eventFilter(watched, event);

    if (auto *line = qobject_cast<QLineEdit*>(w)) {
        showTouchInputPopup(tr("Input"), line->text(), [line](const QString &text) {
            line->setText(text);
        });
        return true;
    }
    if (auto *spin = qobject_cast<QSpinBox*>(w)) {
        showTouchInputPopup(tr("Input Number"), QString::number(spin->value()), [spin](const QString &text) {
            bool ok = false;
            const int v = text.toInt(&ok);
            if (ok)
                spin->setValue(qBound(spin->minimum(), v, spin->maximum()));
        });
        return true;
    }
    if (auto *dspin = qobject_cast<QDoubleSpinBox*>(w)) {
        showTouchInputPopup(tr("Input Number"), QString::number(dspin->value()), [dspin](const QString &text) {
            bool ok = false;
            const double v = text.toDouble(&ok);
            if (ok)
                dspin->setValue(qBound(dspin->minimum(), v, dspin->maximum()));
        });
        return true;
    }
    return QWidget::eventFilter(watched, event);
}

int MainWindow::selectedDataSourceRow() const
{
    if (!ui->treeView || !ui->treeView->selectionModel())
        return -1;

    const QModelIndex current = ui->treeView->selectionModel()->currentIndex();
    if (!current.isValid())
        return -1;

    return current.parent().isValid() ? current.parent().row() : current.row();
}

void MainWindow::updateDataSourceActionButtons()
{
    if (!ui->pushButton_7)
        return;
    ui->pushButton_7->setEnabled(selectedDataSourceRow() >= 0);
}

void MainWindow::refreshDataSourceTree()
{
    if (!m_dataSourceTreeModel || !m_serialDataSource)
        return;

    m_dataSourceTreeModel->removeRows(0, m_dataSourceTreeModel->rowCount());
    for (int i = 0; i < m_modbusConfigs.size(); ++i) {
        const SerialPortConfig cfg = m_modbusConfigs.at(i);
        const QString titlePort = cfg.portName.trimmed().isEmpty() ? QStringLiteral("<unset>") : cfg.portName.trimmed();
        auto *root = new QStandardItem(QString("Modbus RTU %1: %2").arg(i + 1).arg(titlePort));

        const bool isActive = (cfg.deviceId == m_serialDataSource->config().deviceId
                               && cfg.portName == m_serialDataSource->config().portName
                               && cfg.slaveId == m_serialDataSource->config().slaveId);
        root->appendRow(new QStandardItem(QString("Mode: %1")
                                              .arg(m_dataSourceModeCombo ? m_dataSourceModeCombo->text() : "Simulator")));
        root->appendRow(new QStandardItem(QString("Status: %1")
                                              .arg(isActive && m_modbusDataSource && m_modbusDataSource->isOpen() ? "Opened" : "Stopped")));
        root->appendRow(new QStandardItem(QString("Polling: %1")
                                              .arg(isActive && m_modbusDataSource && m_modbusDataSource->isPolling() ? "Running" : "Idle")));
        root->appendRow(new QStandardItem(QString("Baud/Data/Parity/Stop: %1 / %2 / %3 / %4")
                                              .arg(cfg.baudRate)
                                              .arg(static_cast<int>(cfg.dataBits))
                                              .arg(static_cast<int>(cfg.parity))
                                              .arg(static_cast<int>(cfg.stopBits))));
        root->appendRow(new QStandardItem(QString("Slave ID: %1, Timeout: %2 ms, Retry: %3")
                                              .arg(cfg.slaveId)
                                              .arg(cfg.timeoutMs)
                                              .arg(cfg.retryCount)));
        root->appendRow(new QStandardItem(QString("Reserved Poll/FC: %1 ms / %2")
                                              .arg(cfg.pollIntervalMs)
                                              .arg(cfg.defaultFunctionCode)));
        if (isActive && !m_lastCommStatus.isEmpty())
            root->appendRow(new QStandardItem(QString("Last Comm: %1").arg(m_lastCommStatus)));

        m_dataSourceTreeModel->appendRow(root);
    }
    ui->treeView->expandAll();
    updateDataSourceActionButtons();
}

void MainWindow::refreshDataSourceTreeDeferred()
{
    QMetaObject::invokeMethod(this, [this]() { refreshDataSourceTree(); }, Qt::QueuedConnection);
}

void MainWindow::prepareImeForTransientEditor()
{
    QInputMethod *im = QGuiApplication::inputMethod();
    if (!im)
        return;

    im->commit();
    im->hide();
    im->reset();
}

void MainWindow::showSerialConfigDialog()
{
    if (!m_serialDataSource || !m_serialConfigPanel || !m_serialDeviceEdit || !m_serialPortEdit || !m_serialBaudCombo
        || !m_dataBitsCombo || !m_parityCombo || !m_stopBitsCombo || !m_slaveIdSpin
        || !m_timeoutSpin || !m_retrySpin || !m_pollIntervalSpin || !m_functionCodeCombo)
        return;

    prepareImeForTransientEditor();

    m_editingDataSourceRow = selectedDataSourceRow();
    const SerialPortConfig cfg = (m_editingDataSourceRow >= 0 && m_editingDataSourceRow < m_modbusConfigs.size())
                                     ? m_modbusConfigs.at(m_editingDataSourceRow)
                                     : SerialPortConfig{};
    {
        QSignalBlocker b0(m_serialDeviceEdit);
        QSignalBlocker b1(m_serialPortEdit);
        QSignalBlocker b2(m_serialBaudCombo);
        QSignalBlocker b3(m_dataBitsCombo);
        QSignalBlocker b4(m_parityCombo);
        QSignalBlocker b5(m_stopBitsCombo);
        QSignalBlocker b6(m_slaveIdSpin);
        QSignalBlocker b7(m_timeoutSpin);
        QSignalBlocker b8(m_retrySpin);
        QSignalBlocker b9(m_pollIntervalSpin);
        QSignalBlocker b10(m_functionCodeCombo);
        m_serialDeviceEdit->setText(cfg.deviceId);
        m_serialPortEdit->setText(cfg.portName);
        m_serialBaudCombo->setCurrentText(QString::number(cfg.baudRate));
        m_dataBitsCombo->setCurrentText(QString::number(static_cast<int>(cfg.dataBits)));
        m_parityCombo->setCurrentIndex(cfg.parity == QSerialPort::EvenParity ? 1 : (cfg.parity == QSerialPort::OddParity ? 2 : 0));
        m_stopBitsCombo->setCurrentIndex(cfg.stopBits == QSerialPort::TwoStop ? 1 : 0);
        m_slaveIdSpin->setValue(cfg.slaveId);
        m_timeoutSpin->setValue(cfg.timeoutMs);
        m_retrySpin->setValue(cfg.retryCount);
        m_pollIntervalSpin->setValue(cfg.pollIntervalMs);
        switch (cfg.defaultFunctionCode) {
        case 4:
            m_functionCodeCombo->setCurrentIndex(1);
            break;
        case 6:
            m_functionCodeCombo->setCurrentIndex(2);
            break;
        case 16:
            m_functionCodeCombo->setCurrentIndex(3);
            break;
        case 3:
        default:
            m_functionCodeCombo->setCurrentIndex(0);
            break;
        }
    }

    hideDataWorkspacePanels();
    const int panelW = qMin(width() - 32, 920);
    const int panelH = qMin(height() - 32, 520);
    const int panelX = (width() - panelW) / 2;
    const int panelY = (height() - panelH) / 2;
    m_serialConfigPanel->setGeometry(panelX, panelY, panelW, panelH);
    m_serialConfigPanel->show();
    m_serialConfigPanel->raise();

    QTimer::singleShot(0, this, [this]() {
        if (!m_serialPortEdit)
            return;
        m_serialPortEdit->setFocus(Qt::OtherFocusReason);
        QInputMethod *im = QGuiApplication::inputMethod();
        if (im)
            im->show();
    });
}

void MainWindow::hideDataWorkspacePanels()
{
    if (m_serialConfigPanel)
        m_serialConfigPanel->hide();
    if (m_variableEditorPanel)
        m_variableEditorPanel->hide();

    QTimer::singleShot(0, this, [this]() { prepareImeForTransientEditor(); });
}

void MainWindow::fillVariableEditorFromRow(int row)
{
    if (!m_variableModel || row < 0 || row >= m_variableModel->rowCount({}))
        return;

    const Variable var = m_variableModel->variableAt(row);
    m_variableIdEdit->setText(var.id);
    m_variableNameEdit->setText(var.name);
    m_variableDeviceEdit->setText(var.deviceId);
    m_variableTypeCombo->setCurrentText(var.type);
    m_variableAreaCombo->setCurrentText(m_variableModel->data(m_variableModel->index(row, VariableModel::ColArea), Qt::DisplayRole).toString());
    m_variableAddressSpin->setValue(var.address);
    m_variableCountSpin->setValue(qMax(1, var.count));
    m_variableBitOffsetSpin->setValue(qMax(0, var.bitOffset));
    m_variableUnitEdit->setText(var.unit);
    m_variableScaleSpin->setValue(var.scale > 0 ? var.scale : 1.0);
    m_variableReadOnlyCheck->setChecked(var.readOnly);
    m_variableEndianCombo->setCurrentText(m_variableModel->data(m_variableModel->index(row, VariableModel::ColEndianness), Qt::DisplayRole).toString());
}

void MainWindow::showAddVariableDialog()
{
    if (!m_variableEditorPanel)
        return;

    m_variableEditorRow = -1;
    m_variableIdEdit->clear();
    m_variableNameEdit->clear();
    m_variableDeviceEdit->clear();
    m_variableTypeCombo->setCurrentText("float32");
    m_variableAreaCombo->setCurrentText("HoldingRegister");
    m_variableAddressSpin->setValue(0);
    m_variableCountSpin->setValue(1);
    m_variableBitOffsetSpin->setValue(0);
    m_variableUnitEdit->clear();
    m_variableScaleSpin->setValue(1.0);
    m_variableReadOnlyCheck->setChecked(true);
    m_variableEndianCombo->setCurrentText("BigEndian");

    hideDataWorkspacePanels();
    const int panelW = qMin(width() - 32, 920);
    const int panelH = qMin(height() - 32, 560);
    m_variableEditorPanel->setGeometry((width() - panelW) / 2, (height() - panelH) / 2, panelW, panelH);
    m_variableEditorPanel->show();
    m_variableEditorPanel->raise();
}

void MainWindow::showEditVariableDialog()
{
    if (!m_variableEditorPanel || !ui->variableView->selectionModel())
        return;
    const QModelIndex current = ui->variableView->selectionModel()->currentIndex();
    if (!current.isValid())
        return;

    m_variableEditorRow = current.row();
    fillVariableEditorFromRow(m_variableEditorRow);

    hideDataWorkspacePanels();
    const int panelW = qMin(width() - 32, 920);
    const int panelH = qMin(height() - 32, 560);
    m_variableEditorPanel->setGeometry((width() - panelW) / 2, (height() - panelH) / 2, panelW, panelH);
    m_variableEditorPanel->show();
    m_variableEditorPanel->raise();
}

void MainWindow::applyVariableFromPanel()
{
    if (!m_variableModel || !m_variableIdEdit || !m_variableNameEdit)
        return;

    const QString varId = m_variableIdEdit->text().trimmed();
    const QString varName = m_variableNameEdit->text().trimmed();
    if (varId.isEmpty()) {
        showErrorNotice(tr("Variable Config"), tr("Variable ID cannot be empty."));
        return;
    }
    if (varName.isEmpty()) {
        showErrorNotice(tr("Variable Config"), tr("Name cannot be empty."));
        return;
    }
    if (m_variableAddressSpin->value() < 0 || m_variableCountSpin->value() < 1 ||
        m_variableScaleSpin->value() <= 0.0 || m_variableBitOffsetSpin->value() < 0) {
        showErrorNotice(tr("Variable Config"), tr("Invalid numeric fields."));
        return;
    }
    if (m_variableModel->hasVariableId(varId, m_variableEditorRow)) {
        showErrorNotice(tr("Variable Config"), tr("Variable ID already exists."));
        return;
    }

    Variable var;
    if (m_variableEditorRow >= 0 && m_variableEditorRow < m_variableModel->rowCount({}))
        var = m_variableModel->variableAt(m_variableEditorRow);

    var.id = varId;
    var.name = varName;
    var.deviceId = m_variableDeviceEdit->text().trimmed();
    var.type = m_variableTypeCombo->currentText();
    const QString area = m_variableAreaCombo->currentText();
    var.area = area == "Coil" ? RegisterArea::Coil
                              : (area == "DiscreteInput" ? RegisterArea::DiscreteInput
                                                          : (area == "InputRegister" ? RegisterArea::InputRegister : RegisterArea::HoldingRegister));
    var.address = m_variableAddressSpin->value();
    var.count = m_variableCountSpin->value();
    var.bitOffset = m_variableBitOffsetSpin->value();
    var.unit = m_variableUnitEdit->text().trimmed();
    var.scale = m_variableScaleSpin->value();
    var.readOnly = m_variableReadOnlyCheck->isChecked();
    const QString endianText = m_variableEndianCombo->currentText();
    if (endianText == "BigEndianWordSwap")
        var.endianness = Endianness::BigEndianWordSwap;
    else if (endianText == "LittleEndian")
        var.endianness = Endianness::LittleEndian;
    else if (endianText == "LittleEndianByteSwap")
        var.endianness = Endianness::LittleEndianByteSwap;
    else
        var.endianness = Endianness::BigEndian;

    if (m_variableEditorRow >= 0 && m_variableEditorRow < m_variableModel->rowCount({})) {
        m_variableModel->setVariableAt(m_variableEditorRow, var);
    } else {
        m_variableModel->addVariable(var);
        m_variableEditorRow = m_variableModel->rowCount({}) - 1;
    }

    ui->variableView->selectRow(m_variableEditorRow);
    hideDataWorkspacePanels();
    updateVariableActionButtons();
}

void MainWindow::deleteSelectedVariable()
{
    if (!m_variableModel || !ui->variableView->selectionModel())
        return;
    const QModelIndex current = ui->variableView->selectionModel()->currentIndex();
    if (!current.isValid())
        return;
    const int row = current.row();
    if (!m_variableModel->removeVariableAt(row))
        return;
    auto *selection = ui->variableView->selectionModel();
    selection->setCurrentIndex(QModelIndex(), QItemSelectionModel::NoUpdate);
    selection->clearSelection();
    updateVariableActionButtons();
}

void MainWindow::updateVariableActionButtons()
{
    const bool hasSelection = ui->variableView && ui->variableView->selectionModel()
                              && !ui->variableView->selectionModel()->selectedRows().isEmpty();
    ui->pushButton_11->setEnabled(hasSelection);
    ui->pushButton_12->setEnabled(hasSelection);
}

void MainWindow::applySerialConfigFromPanel()
{
    if (!m_serialDataSource || !m_serialDeviceEdit || !m_serialPortEdit || !m_serialBaudCombo
        || !m_dataBitsCombo || !m_parityCombo || !m_stopBitsCombo || !m_slaveIdSpin
        || !m_timeoutSpin || !m_retrySpin || !m_pollIntervalSpin || !m_functionCodeCombo)
        return;

    SerialPortConfig nextCfg;
    nextCfg.deviceId = m_serialDeviceEdit->text().trimmed();
    nextCfg.portName = m_serialPortEdit->text().trimmed();
    nextCfg.baudRate = m_serialBaudCombo->currentText().toInt();
    nextCfg.dataBits = static_cast<QSerialPort::DataBits>(m_dataBitsCombo->currentText().toInt());
    nextCfg.parity = m_parityCombo->currentIndex() == 1
                         ? QSerialPort::EvenParity
                         : (m_parityCombo->currentIndex() == 2 ? QSerialPort::OddParity : QSerialPort::NoParity);
    nextCfg.stopBits = m_stopBitsCombo->currentIndex() == 1 ? QSerialPort::TwoStop : QSerialPort::OneStop;
    nextCfg.slaveId = m_slaveIdSpin->value();
    nextCfg.timeoutMs = m_timeoutSpin->value();
    nextCfg.retryCount = m_retrySpin->value();
    nextCfg.pollIntervalMs = m_pollIntervalSpin->value();
    switch (m_functionCodeCombo->currentIndex()) {
    case 1:
        nextCfg.defaultFunctionCode = 4;
        break;
    case 2:
        nextCfg.defaultFunctionCode = 6;
        break;
    case 3:
        nextCfg.defaultFunctionCode = 16;
        break;
    case 0:
    default:
        nextCfg.defaultFunctionCode = 3;
        break;
    }
    if (m_editingDataSourceRow >= 0 && m_editingDataSourceRow < m_modbusConfigs.size()) {
        m_modbusConfigs[m_editingDataSourceRow] = nextCfg;
    } else {
        m_modbusConfigs.push_back(nextCfg);
        m_editingDataSourceRow = m_modbusConfigs.size() - 1;
    }

    m_serialDataSource->setConfig(nextCfg);
    if (m_modbusDataSource)
        m_modbusDataSource->setConfig(nextCfg);

    if (ui->treeView && ui->treeView->selectionModel()) {
        const QModelIndex idx = m_dataSourceTreeModel->index(m_editingDataSourceRow, 0);
        ui->treeView->selectionModel()->setCurrentIndex(idx, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    }
    m_editingDataSourceRow = -1;

    hideDataWorkspacePanels();
    refreshDataSourceTreeDeferred();
}

void MainWindow::showProperties(CanvasItem *item)
{
    if (!item) return;

    propDiagLog(QString("showProperties enter item=%1 type=%2").arg(reinterpret_cast<quintptr>(item), 0, 16).arg(item->type()));
    QSignalBlocker blocker(ui->propertyTable);

    ui->propertyTable->clear();
    ui->propertyTable->setRowCount(0);
    ui->propertyTable->setColumnCount(2);
    ui->propertyTable->setHorizontalHeaderLabels({"Property", "Value"});

    // 字体增大
    QFont font = ui->propertyTable->font();
    font.setPointSize(14);
    ui->propertyTable->setFont(font);

    QVariantMap props = item->properties();
    QStringList keys = props.keys();
    const QStringList priorityKeys = {
        "title", "text", "value", "varId", "mode", "on", "blink", "threshold", "Interval/Ms",
        "min", "max", "textColor", "onColor", "offColor", "color", "blackBg", "fontSize", "font", "align"
    };
    std::sort(keys.begin(), keys.end(), [&priorityKeys](const QString &a, const QString &b) {
        const int ia = priorityKeys.indexOf(a);
        const int ib = priorityKeys.indexOf(b);
        if (ia >= 0 && ib >= 0) return ia < ib;
        if (ia >= 0) return true;
        if (ib >= 0) return false;
        return a < b;
    });

    int row = 0;
    for (const QString &key : keys) {
        const QVariant propValue = props.value(key);
        ui->propertyTable->insertRow(row);
        QTableWidgetItem *keyItem = new QTableWidgetItem(key);
        QString valueText = propValue.toString();
        if (isColorPropertyKey(key)) {
            if (key == "textColor" || key == "textcolor")
                valueText = normalizedTextColorName(valueText);
            else
                valueText = normalizedColorName(valueText);
        }
        QTableWidgetItem *valueItem = new QTableWidgetItem(valueText);

        keyItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        valueItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
        valueItem->setData(kPropertyValueTypeRole, propValue.type());

        ui->propertyTable->setItem(row, 0, keyItem);

        bool isToggleCell = false;

        if (propValue.type() == QVariant::Bool) {
            valueItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            valueItem->setToolTip("Tap to toggle true / false");
            isToggleCell = true;
        }

        if (key == "blackBg") {
            valueItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            valueItem->setToolTip("Tap to toggle true / false");
            isToggleCell = true;
        }

        if (isColorPropertyKey(key)) {
            valueItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            if (key == "textColor" || key == "textcolor")
                valueItem->setToolTip("Tap to toggle black / red / green / blue / yellow / gray");
            else
                valueItem->setToolTip("Tap to toggle gray / red / green / blue");
            isToggleCell = true;
        }

        if (key == "mode") {
            valueItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            valueItem->setToolTip("Tap to toggle above / below");
            isToggleCell = true;
        }

        if (key == "font") {
            valueItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            valueItem->setToolTip("Tap to toggle normal / italic");
            isToggleCell = true;
        }

        if (key == "align") {
            valueItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            valueItem->setToolTip("Tap to toggle left / right / center");
            isToggleCell = true;
        }

        if (isToggleCell) {
            valueItem->setBackground(QColor(232, 244, 255));
            QFont vf = valueItem->font();
            vf.setBold(true);
            valueItem->setFont(vf);
        }

        ui->propertyTable->setItem(row, 1, valueItem);

        // 设置行高
        ui->propertyTable->setRowHeight(row, 36);  // 调整为触控友好
        row++;
    }

    // 列宽填充，最小宽度
    ui->propertyTable->horizontalHeader()->setStretchLastSection(true);
    ui->propertyTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->propertyTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->propertyTable->horizontalHeader()->setMinimumSectionSize(120);

    // 去掉空白格子显示
    ui->propertyTable->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    ui->propertyTable->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    ui->propertyTable->setShowGrid(true);

    ui->propertyTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    propDiagLog(QString("showProperties done rows=%1").arg(ui->propertyTable->rowCount()));
}



void MainWindow::onItemSelected(CanvasItem *item)
{
    if (m_currentItem && m_currentItem != item)
        m_currentItem->setSelected(false);

    propDiagLog(QString("onItemSelected item=%1").arg(reinterpret_cast<quintptr>(item), 0, 16));
    m_currentItem = item;
    item->setSelected(true);

    showProperties(item);
}



void MainWindow::onPropertyChanged(int row, int col)
{
    if (!m_currentItem || col != 1)
        return;

    QTableWidgetItem *keyCell = ui->propertyTable->item(row, 0);
    QTableWidgetItem *valCell = ui->propertyTable->item(row, 1);
    if (!keyCell || !valCell) {
        propDiagLog(QString("onPropertyChanged skipped invalid cell row=%1 col=%2").arg(row).arg(col));
        return;
    }

    const QString key = keyCell->text();
    const QString val = valCell->text();
    propDiagLog(QString("onPropertyChanged row=%1 key=%2 val=%3 item=%4")
                .arg(row).arg(key).arg(val).arg(reinterpret_cast<quintptr>(m_currentItem), 0, 16));

    m_currentItem->setPropertyValue(key, val);
}


MainWindow::~MainWindow()
{
    if (qApp)
        qApp->removeEventFilter(this);
    delete ui;
}

void MainWindow::on_deleteButton_clicked()
{
    if (!m_currentItem)
        return;

    // 1️⃣ 先通知 CanvasView 清除选中
    if (m_canvas)
        m_canvas->clearSelection();

    // 2️⃣ 延迟安全删除
    m_currentItem->deleteLater();
    m_currentItem = nullptr;

    // 3️⃣ 清空属性表
    clearProperties();
}


void MainWindow::onCanvasEmptyClicked()
{
    // 1. 取消 Canvas 中的选中项
    ui->canvasView->clearSelection();

    // 2. 清空右侧属性表
    ui->propertyTable->clear();
    ui->propertyTable->setRowCount(0);
}

void MainWindow::clearProperties()
{
    ui->propertyTable->clear();
    ui->propertyTable->setRowCount(0);
}

void MainWindow::on_buttonOfFullscreen_clicked()
{
    if (!ui->canvasView) return;
    m_originalStretchList.clear(); // QList<int>，在 MainWindow.h 声明
    if (auto vlayout = qobject_cast<QVBoxLayout*>(m_canvasOriginalLayout)) {
        for (int i = 0; i < vlayout->count(); ++i)
            m_originalStretchList.append(vlayout->stretch(i));
    }


    RuntimeWindow* runWin = new RuntimeWindow(this);
    runWin->setCanvas(ui->canvasView);

    runWin->showFullScreen();
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);

    enforceCanvasFrameRatio();

    refreshActionButtonIcons();
}

void MainWindow::refreshActionButtonIcons()
{
    auto apply = [](QPushButton* btn){
        if (!btn) return;
        const int size = qMin(btn->width(), btn->height());
        if (size < 16)
            return;
        const int iconSize = qMax(16, int(size * 0.97));
        btn->setIconSize(QSize(iconSize, iconSize));
        btn->update();
    };

    // 立即尝试一次，再在布局稳定后补一次，规避时序导致的 setIconSize 失效。
    apply(ui->deleteButton);
    apply(ui->buttonOfFullscreen);
    apply(ui->pushOfL_D);
    apply(ui->pushOfDatasrc);
    apply(ui->pushOfDesign);

    QTimer::singleShot(0, this, [this, apply]() {
        apply(ui->deleteButton);
        apply(ui->buttonOfFullscreen);
        apply(ui->pushOfL_D);
        apply(ui->pushOfDatasrc);
        apply(ui->pushOfDesign);
    });
    QTimer::singleShot(30, this, [this, apply]() {
        apply(ui->deleteButton);
        apply(ui->buttonOfFullscreen);
        apply(ui->pushOfL_D);
        apply(ui->pushOfDatasrc);
        apply(ui->pushOfDesign);
    });
}

void MainWindow::enforceCanvasFrameRatio()
{
    if (!ui || !ui->canvasView || !ui->frame)
        return;

    QWidget *container = ui->canvasView->parentWidget();
    if (!container)
        return;

    int totalHeight = container->height();
    if (ui->widget_2)
        totalHeight = qMax(totalHeight, ui->widget_2->height());
    if (ui->propertyTable && ui->widget_3)
        totalHeight = qMax(totalHeight, ui->propertyTable->height() + ui->widget_3->height());

    if (totalHeight <= 0)
        return;

    const int frameHeight = qMax(1, totalHeight / 4);
    const int canvasHeight = qMax(1, totalHeight - frameHeight);

    // 严格 3:1：无论内部 icon/label 的 sizeHint 如何，都不允许 frame 继续抬高。
    ui->canvasView->setMinimumHeight(canvasHeight);
    ui->canvasView->setMaximumHeight(canvasHeight);
    ui->frame->setMinimumHeight(frameHeight);
    ui->frame->setMaximumHeight(frameHeight);

    ui->canvasView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    ui->frame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    if (auto vlayout = qobject_cast<QVBoxLayout*>(container->layout())) {
        vlayout->setStretch(0, 3);
        vlayout->setStretch(1, 1);
    }
}




void MainWindow::applyCanvasTheme(bool darkMode)
{
    m_darkCanvasMode = darkMode;

    if (!ui || !ui->canvasView)
        return;

    setupIconButton(ui->pushOfL_D, darkMode ? ":/icons/lightmode.png" : ":/icons/darkmode.png");

    if (darkMode) {
        ui->canvasView->setStyleSheet(
            "CanvasView { background-color: #505050; }"
            "CanvasView QWidget { color: #f2f2f2; border-color: #dcdcdc; }"
            "CanvasView QLabel { color: #f2f2f2; }"
            "CanvasView QLineEdit, CanvasView QTextEdit, CanvasView QPlainTextEdit {"
            " color: #f2f2f2; border: 1px solid #d0d0d0; background-color: transparent; }"
            "CanvasView QFrame { border-color: #d0d0d0; }");
    } else {
        ui->canvasView->setStyleSheet(
            "CanvasView { background-color: white; }"
            "CanvasView QWidget { color: black; border-color: black; }"
            "CanvasView QLabel { color: black; }"
            "CanvasView QLineEdit, CanvasView QTextEdit, CanvasView QPlainTextEdit {"
            " color: black; border: 1px solid black; background-color: transparent; }"
            "CanvasView QFrame { border-color: black; }");
    }

    ui->canvasView->update();
    const auto children = ui->canvasView->findChildren<QWidget*>();
    for (QWidget *w : children) {
        if (w)
            w->update();
    }
}

void MainWindow::setupIconButton(QPushButton* btn, const QString& iconPath)
{
    if (!btn) return;

    QPixmap pix(iconPath);
    if (pix.isNull()) return;

    btn->setIcon(QIcon(pix));
    btn->setText(QString());
    btn->setFlat(true);
    btn->setFocusPolicy(Qt::NoFocus);
    btn->setStyleSheet("QPushButton { padding: 0px; margin: 0px; border: none; }");

    // 走统一刷新入口，处理布局时序。
    QTimer::singleShot(0, this, [this]() { refreshActionButtonIcons(); });
}



\

void MainWindow::on_pushOfDatasrc_clicked()
{
    ui->MainStackedWidget->setCurrentWidget(ui->DataWorkspace);
    hideDataWorkspacePanels();
    refreshActionButtonIcons();
    QTimer::singleShot(0, this, [this]() { refreshActionButtonIcons(); });
}

void MainWindow::on_pushOfL_D_clicked()
{
    const bool nextDarkMode = !m_darkCanvasMode;
    applyCanvasTheme(nextDarkMode);
}

void MainWindow::editPropertyCell(int row, int col)
{
    prepareImeForTransientEditor();

    propDiagLog(QString("editPropertyCell click row=%1 col=%2 currentItem=%3")
                .arg(row).arg(col).arg(reinterpret_cast<quintptr>(m_currentItem), 0, 16));
    if (col != 1 || !m_currentItem) return;

    QTableWidgetItem *keyCell = ui->propertyTable->item(row, 0);
    QTableWidgetItem *valueCell = ui->propertyTable->item(row, 1);
    if (!keyCell || !valueCell) return;

    const QString key = keyCell->text();
    const int valueType = valueCell->data(kPropertyValueTypeRole).toInt();
    const QString currentValue = valueCell->text().trimmed().toLower();

    if (isColorPropertyKey(key)) {
        const QString nextColor = (key == "textColor" || key == "textcolor")
                ? nextTextColorName(currentValue)
                : nextColorName(currentValue);
        propDiagLog(QString("editPropertyCell color toggle %1 -> %2 row=%3")
                    .arg(currentValue, nextColor).arg(row));

        if (row >= 0 && row < ui->propertyTable->rowCount()) {
            QTableWidgetItem *latestKeyCell = ui->propertyTable->item(row, 0);
            QTableWidgetItem *latestValueCell = ui->propertyTable->item(row, 1);
            if (latestKeyCell && latestValueCell && latestKeyCell->text() == key) {
                QSignalBlocker blocker(ui->propertyTable);
                latestValueCell->setText(nextColor);
            }
        }

        QPointer<CanvasItem> targetItem = m_currentItem;
        if (targetItem) {
            propDiagLog(QString("apply setPropertyValue(color) target=%1 key=%2 val=%3")
                        .arg(reinterpret_cast<quintptr>(targetItem.data()), 0, 16)
                        .arg(key, nextColor));
            targetItem->setPropertyValue(key, nextColor);
        }
        return;
    }

    if (key == "font" || key == "bold") {
        const QString cur = valueCell->text().trimmed().toLower();
        const QString nextStyle = (cur == "italic") ? "normal" : "italic";

        if (row >= 0 && row < ui->propertyTable->rowCount()) {
            QTableWidgetItem *latestKeyCell = ui->propertyTable->item(row, 0);
            QTableWidgetItem *latestValueCell = ui->propertyTable->item(row, 1);
            if (latestKeyCell && latestValueCell && latestKeyCell->text() == key) {
                QSignalBlocker blocker(ui->propertyTable);
                latestValueCell->setText(nextStyle);
            }
        }

        QPointer<CanvasItem> targetItem = m_currentItem;
        if (targetItem)
            targetItem->setPropertyValue(key == "bold" ? "font" : key, nextStyle);
        return;
    }

    if (key == "align") {
        const QString cur = valueCell->text().trimmed().toLower();
        QString nextAlign = "left";
        if (cur == "left") nextAlign = "right";
        else if (cur == "right") nextAlign = "center";

        if (row >= 0 && row < ui->propertyTable->rowCount()) {
            QTableWidgetItem *latestKeyCell = ui->propertyTable->item(row, 0);
            QTableWidgetItem *latestValueCell = ui->propertyTable->item(row, 1);
            if (latestKeyCell && latestValueCell && latestKeyCell->text() == key) {
                QSignalBlocker blocker(ui->propertyTable);
                latestValueCell->setText(nextAlign);
            }
        }

        QPointer<CanvasItem> targetItem = m_currentItem;
        if (targetItem)
            targetItem->setPropertyValue(key, nextAlign);
        return;
    }

    bool boolTextOk = false;
    const bool currentBoolFromText = parseBoolText(currentValue, &boolTextOk);
    const bool shouldToggleBool = (valueType == QVariant::Bool)
            || (isLikelyBoolPropertyKey(key) && boolTextOk);
    if (shouldToggleBool) {
        const bool currentBool = currentBoolFromText;
        const QString nextBoolText = currentBool ? "false" : "true";
        propDiagLog(QString("editPropertyCell bool toggle key=%1 %2 -> %3 row=%4")
                    .arg(key, currentValue, nextBoolText).arg(row));

        if (row >= 0 && row < ui->propertyTable->rowCount()) {
            QTableWidgetItem *latestKeyCell = ui->propertyTable->item(row, 0);
            QTableWidgetItem *latestValueCell = ui->propertyTable->item(row, 1);
            if (latestKeyCell && latestValueCell && latestKeyCell->text() == key) {
                QSignalBlocker blocker(ui->propertyTable);
                latestValueCell->setText(nextBoolText);
            }
        }

        QPointer<CanvasItem> targetItem = m_currentItem;
        if (targetItem) {
            propDiagLog(QString("apply setPropertyValue(bool) target=%1 key=%2 val=%3")
                        .arg(reinterpret_cast<quintptr>(targetItem.data()), 0, 16)
                        .arg(key, nextBoolText));
            targetItem->setPropertyValue(key, !currentBool);
        }
        return;
    }

    if (key == "mode") {
        const QString modeValue = valueCell->text().trimmed().toLower();
        const QString nextMode = (modeValue == "below") ? "above" : "below";
        propDiagLog(QString("editPropertyCell mode toggle %1 -> %2 row=%3")
                    .arg(modeValue, nextMode).arg(row));

        if (row >= 0 && row < ui->propertyTable->rowCount()) {
            QTableWidgetItem *latestKeyCell = ui->propertyTable->item(row, 0);
            QTableWidgetItem *latestValueCell = ui->propertyTable->item(row, 1);
            if (latestKeyCell && latestValueCell && latestKeyCell->text() == key) {
                QSignalBlocker blocker(ui->propertyTable);
                latestValueCell->setText(nextMode);
            }
        }

        QPointer<CanvasItem> targetItem = m_currentItem;
        if (targetItem) {
            propDiagLog(QString("apply setPropertyValue target=%1 mode=%2")
                        .arg(reinterpret_cast<quintptr>(targetItem.data()), 0, 16).arg(nextMode));
            targetItem->setPropertyValue(key, nextMode);
        } else {
            propDiagLog("skip setPropertyValue: target item destroyed");
        }
        return;
    }

    // 其余属性走输入面板（嵌入式，避免 Wayland 顶层弹窗/IME 生命周期崩溃）
    QInputMethod *im = QGuiApplication::inputMethod();
    if (im) {
        im->commit();
        im->hide();
        im->reset();
    }

    if (!m_propertyInputPanel) {
        QFrame *panel = new QFrame(this);
        panel->setFrameShape(QFrame::StyledPanel);
        panel->setStyleSheet("QFrame { background: white; border: 2px solid #7aa7d9; border-radius: 8px; }");

        QVBoxLayout *layout = new QVBoxLayout(panel);
        m_propertyInputEdit = new QLineEdit(panel);
        m_propertyInputEdit->setAttribute(Qt::WA_InputMethodEnabled, true);
        QFont inputFont = m_propertyInputEdit->font();
        inputFont.setPointSize(18);
        m_propertyInputEdit->setFont(inputFont);
        m_propertyInputEdit->setMinimumHeight(52);
        layout->addWidget(m_propertyInputEdit);

        QPushButton *okBtn = new QPushButton("OK", panel);
        QPushButton *cancelBtn = new QPushButton("Cancel", panel);
        okBtn->setMinimumHeight(40);
        cancelBtn->setMinimumHeight(40);
        layout->addWidget(okBtn);
        layout->addWidget(cancelBtn);

        QObject::connect(okBtn, &QPushButton::clicked, this, [this]() {
            if (!m_propertyInputEdit)
                return;

            const QString newVal = m_propertyInputEdit->text();
            const int row = m_pendingPropertyRow;
            const QString key = m_pendingPropertyKey;
            QPointer<CanvasItem> targetItem = m_pendingPropertyItem;

            if (m_propertyInputPanel)
                m_propertyInputPanel->hide();

            QMetaObject::invokeMethod(this, [this, row, key, newVal, targetItem]() {
                if (row >= 0 && row < ui->propertyTable->rowCount()) {
                    QTableWidgetItem *latestKeyCell = ui->propertyTable->item(row, 0);
                    QTableWidgetItem *latestValueCell = ui->propertyTable->item(row, 1);
                    if (latestKeyCell && latestValueCell && latestKeyCell->text() == key) {
                        QSignalBlocker blocker(ui->propertyTable);
                        latestValueCell->setText(newVal);
                    }
                }
                if (targetItem) {
                    targetItem->setPropertyValue(key, newVal);
                    if (key == "curveCount" || key == "barCount" || key == "varId" || key.startsWith("varId"))
                        showProperties(targetItem);
                }
            }, Qt::QueuedConnection);

            QTimer::singleShot(0, []() {
                QInputMethod *inputMethod = QGuiApplication::inputMethod();
                if (inputMethod) {
                    inputMethod->hide();
                    inputMethod->reset();
                }
            });
        });

        QObject::connect(cancelBtn, &QPushButton::clicked, this, [this]() {
            if (m_propertyInputPanel)
                m_propertyInputPanel->hide();
            QTimer::singleShot(0, []() {
                QInputMethod *inputMethod = QGuiApplication::inputMethod();
                if (inputMethod) {
                    inputMethod->hide();
                    inputMethod->reset();
                }
            });
        });

        panel->hide();
        m_propertyInputPanel = panel;
    }

    m_pendingPropertyItem = m_currentItem;
    m_pendingPropertyKey = key;
    m_pendingPropertyRow = row;

    if (m_propertyInputEdit)
        m_propertyInputEdit->setText(valueCell->text());

    if (m_propertyInputPanel) {
        const int panelW = qMax(320, width() / 2);
        const int panelH = 220;
        const int panelX = (width() - panelW) / 2;
        const int panelY = qMax(16, (height() - panelH) / 2 - height() / 6);
        m_propertyInputPanel->setGeometry(panelX,
                                          panelY,
                                          panelW,
                                          panelH);
        m_propertyInputPanel->show();
        m_propertyInputPanel->raise();
    }

    QTimer::singleShot(0, this, [this]() {
        if (!m_propertyInputEdit)
            return;
        m_propertyInputEdit->setFocus(Qt::OtherFocusReason);
        QInputMethod *inputMethod = QGuiApplication::inputMethod();
        if (inputMethod)
            inputMethod->show();
    });
}

void MainWindow::on_pushOfDesign_clicked()
{
    hideDataWorkspacePanels();
    ui->MainStackedWidget->setCurrentWidget(ui->DesignWorkspace);
    setupIconButton(ui->buttonOfFullscreen, ":/icons/fullscreen.png");
    setupIconButton(ui->deleteButton, ":/icons/delete.png");
    setupIconButton(ui->pushOfDatasrc, ":/icons/datasource.png");
    setupIconButton(ui->pushOfDesign, ":/icons/designmode.png");
    refreshActionButtonIcons();
    QTimer::singleShot(0, this, [this]() { enforceCanvasFrameRatio(); });
}
