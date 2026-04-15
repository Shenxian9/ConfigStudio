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
    // 允许外部在构造时注入绑定管理器，用于后续组件数据绑定。
    setAcceptDrops(true);
    // 与默认构造保持一致：由画布自己负责背景绘制。
    setAttribute(Qt::WA_NoSystemBackground, false);
}

void CanvasView::dragEnterEvent(QDragEnterEvent *event)
{
    // 仅接受携带文本类型标识的拖拽数据（例如 "plot"、"dial"）。
    if (event->mimeData()->hasText())
        event->acceptProposedAction();
}


void CanvasView::dropEvent(QDropEvent *event)
{
    // 从拖拽数据中读取组件类型并创建对应画布项。
    QString type = event->mimeData()->text();
    CanvasItem *item = createItem(type, event->pos());
    if (!item)
        return;

    event->acceptProposedAction();
}


void CanvasView::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    // 按当前缩放比例绘制画布内容。
    QPainter p(this);
    p.scale(m_scale, m_scale);
    p.fillRect(rect(), Qt::white);   // 画白色画布背景

    // 保留样式系统绘制，确保样式表对画布控件仍然生效。
    QStyleOption opt;
    opt.init(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void CanvasView::mousePressEvent(QMouseEvent *event)
{
    // 点击空白处时取消当前选中，并通知属性面板切换到空状态。
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
    // 运行时替换绑定管理器（例如项目加载/切换数据源后）。
    m_bindingMgr = mgr;
    qDebug() << "CanvasView injected bindingMgr =" << m_bindingMgr;
}

QList<CanvasItem*> CanvasView::canvasItems() const
{
    QList<CanvasItem*> items;
    const auto children = findChildren<CanvasItem*>(QString(), Qt::FindDirectChildrenOnly);
    for (CanvasItem *item : children) {
        if (item)
            items.append(item);
    }
    return items;
}

CanvasItem* CanvasView::createItem(const QString &type, const QPoint &pos)
{
    CanvasItem *item = ComponentFactory::create(type, this);
    if (!item) {
        qWarning() << "CanvasView createItem unknown type:" << type;
        return nullptr;
    }
    item->setBindingManager(m_bindingMgr);
    item->move(pos);
    setupItemSignals(item);
    item->show();
    return item;
}

void CanvasView::clearAllItems()
{
    clearSelection();
    const auto items = canvasItems();
    for (CanvasItem *item : items) {
        if (item)
            delete item;
    }
}

void CanvasView::setupItemSignals(CanvasItem *item)
{
    if (!item)
        return;

    connect(item, &CanvasItem::selected, this, [=](CanvasItem* it){
        if (m_selected && m_selected != it)
            m_selected->setSelected(false);
        m_selected = it;
        m_selected->setSelected(true);
        emit itemSelected(it);
    });

    connect(item, &QObject::destroyed, this, [this, item]() {
        if (m_selected == item) {
            m_selected = nullptr;
            emit emptyAreaClicked();
        }
    });
}



