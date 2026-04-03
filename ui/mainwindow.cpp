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
#include <QComboBox>
#include <QSpinBox>
#include <QTableWidget>
#include <QTreeView>
#include <QHeaderView>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QIntValidator>
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

    // ⭐ 1. 表头拉伸铺满
    ui->variableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

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




    qDebug() << "Main model =" << m_variableModel;
    // 添加测试变量（关键是 strategy）
    Variable v1{.id="1", .name="Temp", .deviceId="PLC1", .type="float", .unit="℃",
                .strategy=DataStrategy::Sine};

    Variable v2{.id="2", .name="Pressure", .deviceId="PLC2", .type="float", .unit="Pa",
                .strategy=DataStrategy::Random};
    Variable v3{.id="3", .name="Age", .deviceId="PLC3", .type="float", .unit="year",
                .strategy=DataStrategy::Square};
    Variable v4{.id="Variable", .name="Variable", .deviceId="TEST", .type="float", .unit="",
                .value=50.0, .strategy=DataStrategy::None};

    m_variableModel->addVariable(v1);
    m_variableModel->addVariable(v2);
    m_variableModel->addVariable(v3);
    m_variableModel->addVariable(v4);

    setupDataWorkspace();

    // 启动仿真
    RuntimeSimulator* sim = new RuntimeSimulator(m_variableModel, this);
    sim->start(300);


}

void MainWindow::setupDataWorkspace()
{
    m_serialDataSource = new SerialDataSource(this);
    m_serialMapper = new SerialVariableMapper(m_variableModel, this);
    m_dataSourceTreeModel = new QStandardItemModel(this);

    m_dataSourceTreeModel->setHorizontalHeaderLabels({"Data Sources"});
    ui->treeView->setModel(m_dataSourceTreeModel);
    ui->treeView->header()->setStretchLastSection(true);

    connect(m_serialDataSource, &SerialDataSource::frameReceived,
            m_serialMapper, &SerialVariableMapper::onFrameReceived);

    connect(m_serialDataSource, &SerialDataSource::statusChanged, this, [this](bool opened) {
        ui->pushButton_8->setEnabled(!opened);
        ui->pushButton_9->setEnabled(opened);
        refreshDataSourceTreeDeferred();
    });

    connect(m_serialDataSource, &SerialDataSource::errorOccurred, this, [this](const QString &err) {
        QMessageBox::warning(this, tr("Modbus RTU Data Source"), err);
        refreshDataSourceTreeDeferred();
    });

    connect(ui->pushButton_2, &QPushButton::clicked, this, &MainWindow::showSerialConfigDialog);
    connect(ui->pushButton_8, &QPushButton::clicked, this, [this]() {
        if (!m_serialDataSource->open())
            return;
        refreshDataSourceTreeDeferred();
    });
    connect(ui->pushButton_9, &QPushButton::clicked, this, [this]() {
        m_serialDataSource->close();
        refreshDataSourceTreeDeferred();
    });
    connect(ui->pushButton_7, &QPushButton::clicked, this, [this]() {
        m_serialDataSource->close();
        m_serialMapper->clearBindings();
        SerialPortConfig cfg;
        m_serialDataSource->setConfig(cfg);
        refreshDataSourceTreeDeferred();
    });

    connect(ui->pushButton_10, &QPushButton::clicked, this, [this]() { showMappingDialog(); });
    connect(ui->pushButton_11, &QPushButton::clicked, this, [this]() { showMappingDialog(); });
    connect(ui->pushButton_12, &QPushButton::clicked, this, [this]() { showMappingDialog(); });

    ui->pushButton_2->setText("Add/Config Modbus RTU");
    ui->pushButton_7->setText("Remove Modbus RTU");
    ui->pushButton_10->setText("Add/Edit Mapping");
    ui->pushButton_11->setText("Delete Mapping");
    ui->pushButton_12->setText("Data Mapping");

    ui->pushButton_8->setEnabled(true);
    ui->pushButton_9->setEnabled(false);

    setupDataWorkspacePanels();
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

        m_serialPortEdit = new QLineEdit(m_serialConfigPanel);
        m_serialPortEdit->setPlaceholderText("/dev/ttyS2 or COM3");
        m_serialBaudCombo = new QComboBox(m_serialConfigPanel);
        m_serialBaudCombo->addItems({"1200", "2400", "4800", "9600", "19200", "38400", "57600", "115200"});
        m_dataBitsCombo = new QComboBox(m_serialConfigPanel);
        m_dataBitsCombo->addItems({"5", "6", "7", "8"});
        m_parityCombo = new QComboBox(m_serialConfigPanel);
        m_parityCombo->addItems({"None", "Even", "Odd"});
        m_stopBitsCombo = new QComboBox(m_serialConfigPanel);
        m_stopBitsCombo->addItems({"1", "2"});

        serialForm->addRow("Port", m_serialPortEdit);
        serialForm->addRow("BaudRate", m_serialBaudCombo);
        serialForm->addRow("DataBits", m_dataBitsCombo);
        serialForm->addRow("Parity", m_parityCombo);
        serialForm->addRow("StopBits", m_stopBitsCombo);
        layout->addWidget(serialGroup);

        auto *modbusGroup = new QGroupBox("Modbus Parameters", m_serialConfigPanel);
        auto *modbusForm = new QFormLayout(modbusGroup);
        modbusForm->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
        modbusForm->setSpacing(10);

        m_slaveIdSpin = new QSpinBox(m_serialConfigPanel);
        m_slaveIdSpin->setRange(1, 247);
        m_timeoutSpin = new QSpinBox(m_serialConfigPanel);
        m_timeoutSpin->setRange(50, 60000);
        m_timeoutSpin->setSingleStep(50);
        m_retrySpin = new QSpinBox(m_serialConfigPanel);
        m_retrySpin->setRange(0, 10);
        m_pollIntervalSpin = new QSpinBox(m_serialConfigPanel);
        m_pollIntervalSpin->setRange(50, 60000);
        m_pollIntervalSpin->setSingleStep(50);
        m_functionCodeCombo = new QComboBox(m_serialConfigPanel);
        m_functionCodeCombo->addItems({"03 - Read Holding Registers", "04 - Read Input Registers", "06 - Write Single Register", "10 - Write Multiple Registers"});

        modbusForm->addRow("Slave ID", m_slaveIdSpin);
        modbusForm->addRow("Timeout (ms)", m_timeoutSpin);
        modbusForm->addRow("Retry Count", m_retrySpin);
        modbusForm->addRow("Poll Interval (ms)", m_pollIntervalSpin);
        modbusForm->addRow("Default Function Code", m_functionCodeCombo);
        layout->addWidget(modbusGroup);

        auto *hint = new QLabel("Register table and actual Modbus transactions will be added in a later step.", m_serialConfigPanel);
        hint->setWordWrap(true);
        hint->setStyleSheet("color:#666;");
        layout->addWidget(hint);

        auto *buttons = new QHBoxLayout();
        auto *testBtn = new QPushButton("Test Connection", m_serialConfigPanel);
        testBtn->setEnabled(false);
        auto *okBtn = new QPushButton("Apply", m_serialConfigPanel);
        auto *cancelBtn = new QPushButton("Cancel", m_serialConfigPanel);
        buttons->addStretch();
        buttons->addWidget(testBtn);
        buttons->addWidget(okBtn);
        buttons->addWidget(cancelBtn);
        layout->addLayout(buttons);

        connect(okBtn, &QPushButton::clicked, this, &MainWindow::applySerialConfigFromPanel);
        connect(cancelBtn, &QPushButton::clicked, this, &MainWindow::hideDataWorkspacePanels);
    }

    if (!m_mappingPanel) {
        m_mappingPanel = new QFrame(this);
        m_mappingPanel->setObjectName("mappingPanel");
        m_mappingPanel->setStyleSheet("#mappingPanel { background: #f7f7f7; border: 1px solid #888; border-radius: 6px; }");
        m_mappingPanel->hide();

        auto *layout = new QVBoxLayout(m_mappingPanel);
        layout->setContentsMargins(12, 12, 12, 12);
        layout->setSpacing(8);

        auto *title = new QLabel("Modbus Key -> Variable Mapping", m_mappingPanel);
        layout->addWidget(title);

        m_mappingTable = new QTableWidget(m_mappingPanel);
        m_mappingTable->setColumnCount(2);
        m_mappingTable->setHorizontalHeaderLabels({"Source Key", "Variable ID"});
        m_mappingTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        m_mappingTable->setAttribute(Qt::WA_InputMethodEnabled, false);
        layout->addWidget(m_mappingTable);

        auto *rowButtons = new QHBoxLayout();
        auto *addBtn = new QPushButton("Add Row", m_mappingPanel);
        auto *delBtn = new QPushButton("Delete Row", m_mappingPanel);
        rowButtons->addWidget(addBtn);
        rowButtons->addWidget(delBtn);
        rowButtons->addStretch();
        layout->addLayout(rowButtons);

        connect(addBtn, &QPushButton::clicked, this, [this]() {
            if (!m_mappingTable)
                return;
            const int newRow = m_mappingTable->rowCount();
            m_mappingTable->insertRow(newRow);
            auto *keyItem = new QTableWidgetItem(QString());
            auto *idItem = new QTableWidgetItem(QString());
            keyItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
            idItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
            m_mappingTable->setItem(newRow, 0, keyItem);
            m_mappingTable->setItem(newRow, 1, idItem);
        });

        connect(delBtn, &QPushButton::clicked, this, [this]() {
            if (!m_mappingTable)
                return;
            const int row = m_mappingTable->currentRow();
            if (row >= 0)
                m_mappingTable->removeRow(row);
        });

        auto *buttons = new QHBoxLayout();
        auto *okBtn = new QPushButton("Apply", m_mappingPanel);
        auto *cancelBtn = new QPushButton("Cancel", m_mappingPanel);
        buttons->addStretch();
        buttons->addWidget(okBtn);
        buttons->addWidget(cancelBtn);
        layout->addLayout(buttons);

        connect(okBtn, &QPushButton::clicked, this, &MainWindow::applyMappingFromPanel);
        connect(cancelBtn, &QPushButton::clicked, this, &MainWindow::hideDataWorkspacePanels);
    }
}

void MainWindow::refreshDataSourceTree()
{
    if (!m_dataSourceTreeModel || !m_serialDataSource || !m_serialMapper)
        return;

    m_dataSourceTreeModel->removeRows(0, m_dataSourceTreeModel->rowCount());

    const SerialPortConfig cfg = m_serialDataSource->config();
    auto *root = new QStandardItem(QString("Modbus RTU: %1").arg(cfg.portName));

    root->appendRow(new QStandardItem(QString("Status: %1").arg(m_serialDataSource->isOpen() ? "Running" : "Stopped")));
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

    auto *mappingRoot = new QStandardItem("Mappings");
    const auto mappings = m_serialMapper->bindings();
    for (auto it = mappings.cbegin(); it != mappings.cend(); ++it)
        mappingRoot->appendRow(new QStandardItem(QString("%1 -> %2").arg(it.key(), it.value())));
    root->appendRow(mappingRoot);

    m_dataSourceTreeModel->appendRow(root);
    ui->treeView->expandAll();
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
    if (!m_serialDataSource || !m_serialConfigPanel || !m_serialPortEdit || !m_serialBaudCombo
        || !m_dataBitsCombo || !m_parityCombo || !m_stopBitsCombo || !m_slaveIdSpin
        || !m_timeoutSpin || !m_retrySpin || !m_pollIntervalSpin || !m_functionCodeCombo)
        return;

    prepareImeForTransientEditor();

    const SerialPortConfig cfg = m_serialDataSource->config();
    {
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
    const int panelW = qMin(width() - 32, 620);
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

void MainWindow::showMappingDialog()
{
    if (!m_serialMapper || !m_variableModel || !m_mappingPanel || !m_mappingTable)
        return;

    prepareImeForTransientEditor();

    {
        QSignalBlocker blocker(m_mappingTable);
        m_mappingTable->setRowCount(0);
    }

    const auto existing = m_serialMapper->bindings();
    int row = 0;
    for (auto it = existing.cbegin(); it != existing.cend(); ++it) {
        m_mappingTable->insertRow(row);
        m_mappingTable->setItem(row, 0, new QTableWidgetItem(it.key()));
        m_mappingTable->setItem(row, 1, new QTableWidgetItem(it.value()));
        ++row;
    }

    for (int r = 0; r < m_mappingTable->rowCount(); ++r) {
        auto *keyItem = m_mappingTable->item(r, 0);
        auto *idItem = m_mappingTable->item(r, 1);
        if (keyItem)
            keyItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
        if (idItem)
            idItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
    }

    hideDataWorkspacePanels();
    const int panelW = qMin(width() - 32, 640);
    const int panelH = qMin(height() - 32, 460);
    const int panelX = (width() - panelW) / 2;
    const int panelY = (height() - panelH) / 2;
    m_mappingPanel->setGeometry(panelX, panelY, panelW, panelH);
    m_mappingPanel->show();
    m_mappingPanel->raise();

    QTimer::singleShot(0, this, [this]() {
        if (!m_mappingTable)
            return;
        m_mappingTable->setFocus(Qt::OtherFocusReason);
    });
}

void MainWindow::hideDataWorkspacePanels()
{
    if (m_serialConfigPanel)
        m_serialConfigPanel->hide();
    if (m_mappingPanel)
        m_mappingPanel->hide();

    QTimer::singleShot(0, this, [this]() { prepareImeForTransientEditor(); });
}

void MainWindow::applySerialConfigFromPanel()
{
    if (!m_serialDataSource || !m_serialPortEdit || !m_serialBaudCombo
        || !m_dataBitsCombo || !m_parityCombo || !m_stopBitsCombo || !m_slaveIdSpin
        || !m_timeoutSpin || !m_retrySpin || !m_pollIntervalSpin || !m_functionCodeCombo)
        return;

    SerialPortConfig nextCfg = m_serialDataSource->config();
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
    m_serialDataSource->setConfig(nextCfg);

    hideDataWorkspacePanels();
    refreshDataSourceTreeDeferred();
}

void MainWindow::applyMappingFromPanel()
{
    if (!m_serialMapper || !m_mappingTable)
        return;

    m_serialMapper->clearBindings();
    for (int r = 0; r < m_mappingTable->rowCount(); ++r) {
        const QTableWidgetItem *keyItem = m_mappingTable->item(r, 0);
        const QTableWidgetItem *idItem = m_mappingTable->item(r, 1);
        if (!keyItem || !idItem)
            continue;
        const QString key = keyItem->text().trimmed();
        const QString id = idItem->text().trimmed();
        if (!key.isEmpty() && !id.isEmpty())
            m_serialMapper->setBinding(key, id);
    }

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
