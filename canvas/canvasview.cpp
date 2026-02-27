#include "canvasview.h"

#include <QDragEnterEvent>
#include <QMimeData>
#include "components/base/ComponentFactory.h"

CanvasView::CanvasView(QWidget *parent)
    : QWidget(parent)
{
    // 允许从组件面板拖拽控件到画布。
    setAcceptDrops(true);

    // 关闭系统默认背景，交给 paintEvent 统一绘制。
    setAttribute(Qt::WA_NoSystemBackground, false);
    m_canvasViewOriginalSize = size();


}

CanvasView::CanvasView(DataBindingManager* bindingMgr, QWidget *parent)
    : QWidget(parent),
    m_bindingMgr(bindingMgr)
{
    setAcceptDrops(true);
    setAttribute(Qt::WA_NoSystemBackground, false);
}

void CanvasView::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasText())
        event->acceptProposedAction();
}


void CanvasView::dropEvent(QDropEvent *event)
{
    // 从拖拽数据中读取组件类型并创建对应画布项。
    QString type = event->mimeData()->text();
    CanvasItem *item = ComponentFactory::create(type, this);
    if (!item) return;

    // ⭐ 注入绑定管理器
    item->setBindingManager(m_bindingMgr);

    // 将组件放到鼠标释放位置并显示。
    item->move(event->pos());
    item->show();

    // 绑定测试变量（可用属性表修改 varId）
    // if (m_bindingMgr) {
    //     if (type == "thermo") {
    //         m_bindingMgr->bind("1", item, "value");
    //     } else if (type == "plot") {
    //         m_bindingMgr->bind("2", item, "value");
    //     }
    // }

    // 维护单选状态：新选中项会取消旧选中项。
    connect(item, &CanvasItem::selected, this, [=](CanvasItem* it){
        if (m_selected && m_selected != it)
            m_selected->setSelected(false);
        m_selected = it;
        m_selected->setSelected(true);
        emit itemSelected(it);
    });

    // 组件被删除时同步清空画布当前选中状态。
    connect(item, &QObject::destroyed, this, [this, item]() {
        if (m_selected == item) {
            m_selected = nullptr;
            emit emptyAreaClicked();
        }
    });

    event->acceptProposedAction();
}


void CanvasView::paintEvent(QPaintEvent *event)
{
    QPainter p(this);
    p.scale(m_scale, m_scale);
    p.fillRect(rect(), Qt::white);   // 画白色画布背景

    QStyleOption opt;
    opt.init(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void CanvasView::mousePressEvent(QMouseEvent *event)
{
    QWidget *child = childAt(event->pos());

    CanvasItem *item = qobject_cast<CanvasItem*>(child);

    if (!item) {
        // 点在空白处
        clearSelection();
        emit emptyAreaClicked();
    }

    QWidget::mousePressEvent(event);
}


void CanvasView::clearSelection()
{
    // 对外提供统一的取消选中入口。
    if (m_selected) {
        m_selected->setSelected(false);
        m_selected = nullptr;
    }
}

void CanvasView::setBindingManager(DataBindingManager* mgr)
{
    m_bindingMgr = mgr;
    qDebug() << "CanvasView injected bindingMgr =" << m_bindingMgr;
}





