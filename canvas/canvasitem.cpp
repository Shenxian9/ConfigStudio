#include "canvasitem.h"
#include <QPainter>

CanvasItem::CanvasItem(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);

    m_selectionFrame = new QFrame(this);
    m_selectionFrame->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    m_selectionFrame->setStyleSheet("border: 2px dashed rgb(40,120,255); background: transparent;");
    m_selectionFrame->hide();

    m_resizeHandleOverlay = new QWidget(this);
    m_resizeHandleOverlay->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    m_resizeHandleOverlay->setStyleSheet("background: rgb(40,120,255);");
    m_resizeHandleOverlay->hide();

    updateSelectionOverlay();
}

void CanvasItem::setSelected(bool sel)
{
    m_selected = sel;
    updateSelectionOverlay();
    update();
}

void CanvasItem::updateSelectionOverlay()
{
    if (!m_selectionFrame || !m_resizeHandleOverlay)
        return;

    if (!m_selected) {
        m_selectionFrame->hide();
        m_resizeHandleOverlay->hide();
        return;
    }

    QRect frameRect = rect().adjusted(1, 1, -1, -1);
    if (frameRect.width() < 4 || frameRect.height() < 4)
        frameRect = rect();

    m_selectionFrame->setGeometry(frameRect);
    m_selectionFrame->show();
    m_selectionFrame->raise();

    QRect handle(rect().right() - handleSize, rect().bottom() - handleSize, handleSize, handleSize);
    m_resizeHandleOverlay->setGeometry(handle);
    m_resizeHandleOverlay->show();
    m_resizeHandleOverlay->raise();
}

void CanvasItem::setEditLocked(bool locked)
{
    m_editLocked = locked;
    if (locked) {
        m_dragging = false;
        m_resizing = false;
    }

    const QList<QWidget*> widgets = findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly);
    for (QWidget *w : widgets) {
        if (!w || w == m_selectionFrame || w == m_resizeHandleOverlay)
            continue;
        w->setAttribute(Qt::WA_TransparentForMouseEvents, !locked);
    }
    updateSelectionOverlay();
    update();
}

void CanvasItem::childEvent(QChildEvent *event)
{
    QWidget::childEvent(event);

    if (!event || !event->added())
        return;

    QWidget *w = qobject_cast<QWidget*>(event->child());
    if (!w || w == m_selectionFrame || w == m_resizeHandleOverlay)
        return;

    // 设计态时子控件不消费鼠标事件，便于选中/拖拽组件；
    // 运行态（editLocked=true）恢复子控件交互。
    w->setAttribute(Qt::WA_TransparentForMouseEvents, !m_editLocked);
}

void CanvasItem::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateSelectionOverlay();
}

void CanvasItem::mousePressEvent(QMouseEvent *event)
{
    if (m_editLocked) {
        QWidget::mousePressEvent(event);
        return;
    }

    if (event->button() != Qt::LeftButton)
        return;

    emit selected(this);

    if (isInResizeHandle(event->pos())) {
        m_resizing = true;
        m_dragStart = event->globalPos();
        m_startRect = rect();
    }
    else {
        m_dragging = true;
        m_dragStart = event->globalPos();
        m_startRect = geometry();
    }

    event->accept();
}

void CanvasItem::mouseMoveEvent(QMouseEvent *event)
{
    if (m_editLocked) {
        QWidget::mouseMoveEvent(event);
        return;
    }

    QPoint delta = event->globalPos() - m_dragStart;

    if (m_resizing) {
        int newWidth = std::max(20, m_startRect.width() + delta.x());
        int newHeight = std::max(20, m_startRect.height() + delta.y());
        resize(newWidth, newHeight);
    }
    else if (m_dragging) {
        move(m_startRect.topLeft() + delta);
    }

    QWidget::mouseMoveEvent(event);
}

void CanvasItem::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
}

bool CanvasItem::isInResizeHandle(const QPoint& pos) const
{
    QRect handle(rect().right()-handleSize, rect().bottom()-handleSize, handleSize, handleSize);
    return handle.contains(pos);
}

void CanvasItem::mouseReleaseEvent(QMouseEvent *event)
{
    if (!m_editLocked) {
        m_resizing = false;
        m_dragging = false;
    }
    QWidget::mouseReleaseEvent(event);
}
