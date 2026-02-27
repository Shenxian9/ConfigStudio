#include "componentpalette.h"

#include <QDrag>
#include <QMimeData>
#include <QPainter>
#include <QMouseEvent>


ComponentPalette::ComponentPalette(const QString& type, QWidget *parent)
    : QLabel(parent), m_type(type)
{
    // setFixedSize(32, 32);

    // // 这里将来用真实图标，现在用红块
    // QPixmap pixmap(size());
    // pixmap.fill(Qt::transparent);

    // QPainter p(&pixmap);
    // p.setBrush(Qt::red);
    // p.setPen(Qt::black);
    // p.drawRect(rect().adjusted(0,0,-1,-1));
    // p.end();

    // setPixmap(pixmap);
}

void ComponentPalette::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton)
        return;

    QDrag *drag = new QDrag(this);
    QMimeData *mime = new QMimeData;

    // 只传“类型”
    mime->setText(m_type);
    drag->setMimeData(mime);

    drag->setPixmap(pixmap(Qt::ReturnByValue));
    drag->setHotSpot(event->pos());

    drag->exec(Qt::CopyAction);
}
