#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QSignalBlocker>
#include <QPointer>
#include <QLoggingCategory>
#include <QDateTime>
#include <QColor>
#include <QInputMethod>
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

QString nextColorName(const QString &current)
{
    const QString c = normalizedColorName(current);
    if (c == "gray") return "red";
    if (c == "red") return "green";
    if (c == "green") return "blue";
    return "gray";
}

bool isColorPropertyKey(const QString &key)
{
    return key == "color" || key == "offColor";
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

    m_variableModel->addVariable(v1);
    m_variableModel->addVariable(v2);
        m_variableModel->addVariable(v3);

    // 启动仿真
    RuntimeSimulator* sim = new RuntimeSimulator(m_variableModel, this);
    sim->start(300);


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
        "min", "max", "color", "fontSize", "bold", "align"
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

        if (isColorPropertyKey(key)) {
            valueItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            valueItem->setToolTip("Tap to toggle gray / red / green / blue");
            isToggleCell = true;
        }

        if (key == "mode") {
            valueItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            valueItem->setToolTip("Tap to toggle above / below");
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

    auto update = [](QPushButton* btn){
        if (!btn) return;
        int size = qMin(btn->width(), btn->height()); // 以最短边为准
        btn->setIconSize(QSize(size, size));
    };

    update(ui->deleteButton);
    update(ui->buttonOfFullscreen);
}




void MainWindow::setupIconButton(QPushButton* btn, const QString& iconPath)
{
    if (!btn) return;

    QPixmap pix(iconPath);
    if (pix.isNull()) return;

    btn->setIcon(QIcon(pix));
    btn->setFlat(true);
    btn->setFocusPolicy(Qt::NoFocus);

    // 延迟设置 iconSize
    QTimer::singleShot(0, btn, [btn](){
        int size = qMin(btn->width(), btn->height());
        if (size > 0)
            btn->setIconSize(QSize(size, size));
    });
}



\

void MainWindow::on_pushOfDatasrc_clicked()
{
    ui->MainStackedWidget->setCurrentWidget(ui->DataWorkspace);
}

void MainWindow::editPropertyCell(int row, int col)
{
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
        const QString nextColor = nextColorName(currentValue);
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

    if (valueType == QVariant::Bool) {
        const bool currentBool = (currentValue == "true" || currentValue == "1");
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
                if (targetItem)
                    targetItem->setPropertyValue(key, newVal);
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
    ui->MainStackedWidget->setCurrentWidget(ui->DesignWorkspace);
    setupIconButton(ui->buttonOfFullscreen, ":/icons/fullscreen.png");
    setupIconButton(ui->deleteButton, ":/icons/delete.png");
}
