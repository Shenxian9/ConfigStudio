#include "runtimewindow.h"
#include "ui_runtimewindow.h"
#include "canvas/canvasview.h"

RuntimeWindow::RuntimeWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::RuntimeWindow)
{
    ui->setupUi(this);

    setupIconButton(ui->buttonOfQuit, ":/icons/exit.png");

}

RuntimeWindow::~RuntimeWindow()
{
    ui->centralWidget->layout()->removeWidget(m_canvas);
    // 不删除 m_canvas，交还给原父控件
    if (m_canvas)
        m_canvas->setParent(m_originalParent);
    delete ui;
}

void RuntimeWindow::setCanvas(CanvasView* canvas)
{
    if (!canvas) return;
    m_canvas = canvas;

    // 保存原父控件和布局
    m_originalParent = canvas->parentWidget();
    m_originalLayout = m_originalParent->layout();
    m_originalIndex = m_originalLayout ? m_originalLayout->indexOf(canvas) : -1;
    m_originalSize = canvas->size();
    m_originalPolicy = canvas->sizePolicy();
    m_originalMinSize = canvas->minimumSize();
    m_originalMaxSize = canvas->maximumSize();

    if (auto vlayout = qobject_cast<QVBoxLayout*>(m_originalLayout)) {
        if (m_originalIndex != -1)
            m_originalStretch = vlayout->stretch(m_originalIndex);
    }

    // 保存 CanvasItem 原始 geometry
    m_originalItemGeometries.clear();
    for (auto item : m_canvas->findChildren<CanvasItem*>()) {
        m_originalItemGeometries[item] = item->geometry();
    }

    // 运行态只观察，不允许拖拽/缩放/选中
    m_canvas->clearSelection();
    for (auto item : m_canvas->findChildren<CanvasItem*>()) {
        item->setSelected(false);
        item->setEditLocked(true);
    }

    // 移动到 RuntimeWindow
    canvas->setParent(ui->centralWidget);
    if (!ui->centralWidget->layout())
        ui->centralWidget->setLayout(new QVBoxLayout);
    ui->centralWidget->layout()->addWidget(canvas);
    ui->centralWidget->layout()->setContentsMargins(0,0,0,0);

    // 进入全屏时解除设计态比例约束，让 Canvas 填满运行窗口。
    canvas->setMinimumSize(0, 0);
    canvas->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    canvas->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    applyRuntimeScale();
    canvas->show();
}


void RuntimeWindow::on_buttonOfQuit_clicked()
{
    if (!m_canvas) return;

    // 恢复 CanvasView 父控件和布局
    m_canvas->setParent(m_originalParent);
    if (auto vlayout = qobject_cast<QVBoxLayout*>(m_originalLayout)) {
        if (m_originalIndex != -1)
            vlayout->insertWidget(m_originalIndex, m_canvas);
        vlayout->setStretch(m_originalIndex, m_originalStretch);
    }

    m_canvas->setSizePolicy(m_originalPolicy);
    m_canvas->setMinimumSize(m_originalMinSize);
    m_canvas->setMaximumSize(m_originalMaxSize);
    m_canvas->resize(m_originalSize);

    // 恢复 CanvasItem geometry
    for (auto item : m_canvas->findChildren<CanvasItem*>()) {
        if (m_originalItemGeometries.contains(item)) {
            item->setGeometry(m_originalItemGeometries[item]);

            // 同时恢复 IconLabel 的 pixmap
            for (auto icon : item->findChildren<IconLabel*>()) {
                icon->updatePixmap();
            }
        }
        // 恢复设计态编辑能力
        item->setEditLocked(false);
    }


    // 直接刷新 CanvasView
    m_canvas->update();


    close();
}

void RuntimeWindow::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);

    applyRuntimeScale();

    auto update = [](QPushButton* btn){
        if (btn)
            btn->setIconSize(btn->size());
    };

    update(ui->buttonOfQuit);

}

void RuntimeWindow::applyRuntimeScale()
{
    if (!m_canvas || !ui || !ui->centralWidget)
        return;

    const QSize fullSize = ui->centralWidget->size();
    if (fullSize.isEmpty() || m_originalSize.isEmpty())
        return;

    const double sx = double(fullSize.width()) / m_originalSize.width();
    const double sy = double(fullSize.height()) / m_originalSize.height();

    m_canvas->resize(fullSize);
    m_canvas->move(0, 0);

    for (auto item : m_canvas->findChildren<CanvasItem*>()) {
        QRect g = m_originalItemGeometries.value(item, item->geometry());
        item->setGeometry(int(g.x() * sx), int(g.y() * sy), int(g.width() * sx), int(g.height() * sy));

        for (auto icon : item->findChildren<IconLabel*>()) {
            icon->updatePixmap();
        }
    }
}

void RuntimeWindow::setupIconButton(QPushButton* btn, const QString& iconPath)
{
    if (!btn) return;

    QPixmap pix(iconPath);
    if (pix.isNull()) return;

    btn->setIcon(QIcon(pix));
    btn->setIconSize(btn->size());
    btn->setFlat(true);                 // 可选
    btn->setFocusPolicy(Qt::NoFocus);   // 可选
}
