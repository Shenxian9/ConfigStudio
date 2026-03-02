#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QSignalBlocker>
#include <QPointer>
#include <QLoggingCategory>
#include <QDateTime>

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
    int row = 0;
    for (auto it = props.begin(); it != props.end(); ++it) {
        const QString key = it.key();
        ui->propertyTable->insertRow(row);
        QTableWidgetItem *keyItem = new QTableWidgetItem(key);
        QTableWidgetItem *valueItem = new QTableWidgetItem(it.value().toString());

        keyItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        valueItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);

        ui->propertyTable->setItem(row, 0, keyItem);

        if (key == "blinkMode") {
            valueItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            valueItem->setToolTip("Tap to choose: above / below");
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
    if (key != "blinkMode") {
        return;
    }
    propDiagLog(QString("editPropertyCell blinkMode current=%1 row=%2").arg(valueCell->text()).arg(row));

    const QString currentValue = valueCell->text();
    QPointer<CanvasItem> targetItem = m_currentItem;

    QDialog dlg(this);
    dlg.setWindowTitle("选择闪烁模式");

    QVBoxLayout *lay = new QVBoxLayout(&dlg);
    QPushButton *above = new QPushButton("above", &dlg);
    QPushButton *below = new QPushButton("below", &dlg);

    above->setMinimumHeight(44);
    below->setMinimumHeight(44);

    lay->addWidget(above);
    lay->addWidget(below);

    QString selectedMode;
    connect(above, &QPushButton::clicked, &dlg, [&selectedMode, &dlg](){
        selectedMode = "above";
        dlg.accept();
    });

    connect(below, &QPushButton::clicked, &dlg, [&selectedMode, &dlg](){
        selectedMode = "below";
        dlg.accept();
    });

    if (currentValue == "above") {
        above->setFocus();
    } else {
        below->setFocus();
    }

    const int dlgRet = dlg.exec();
    propDiagLog(QString("blinkMode dialog ret=%1 selected=%2").arg(dlgRet).arg(selectedMode));
    if (dlgRet != QDialog::Accepted || selectedMode.isEmpty()) {
        return;
    }

    if (row >= 0 && row < ui->propertyTable->rowCount()) {
        QTableWidgetItem *latestKeyCell = ui->propertyTable->item(row, 0);
        QTableWidgetItem *latestValueCell = ui->propertyTable->item(row, 1);
        if (latestKeyCell && latestValueCell && latestKeyCell->text() == key) {
            QSignalBlocker blocker(ui->propertyTable);
            propDiagLog(QString("apply table text row=%1 mode=%2").arg(row).arg(selectedMode));
            latestValueCell->setText(selectedMode);
        }
    }

    if (targetItem) {
        propDiagLog(QString("apply setPropertyValue target=%1 mode=%2")
                    .arg(reinterpret_cast<quintptr>(targetItem.data()), 0, 16).arg(selectedMode));
        targetItem->setPropertyValue(key, selectedMode);
    } else {
        propDiagLog("skip setPropertyValue: target item destroyed");
    }
}

void MainWindow::on_pushOfDesign_clicked()
{
    ui->MainStackedWidget->setCurrentWidget(ui->DesignWorkspace);
    setupIconButton(ui->buttonOfFullscreen, ":/icons/fullscreen.png");
    setupIconButton(ui->deleteButton, ":/icons/delete.png");
}
