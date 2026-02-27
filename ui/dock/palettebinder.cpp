#include "palettebinder.h"

#include <QMouseEvent>
#include <QDrag>
#include <QMimeData>

PaletteBinder::PaletteBinder(QObject *parent)
    : QObject(parent)
{
}

void PaletteBinder::bind(QLabel *label, const QString &type)
{
    m_map[label] = type;
    label->installEventFilter(this);
}

bool PaletteBinder::eventFilter(QObject *obj, QEvent *event)
{
    QLabel *label = qobject_cast<QLabel*>(obj);
    if (!label || !m_map.contains(label))
        return QObject::eventFilter(obj, event);

    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *me = static_cast<QMouseEvent*>(event);
        if (me->button() == Qt::LeftButton) {

            QDrag *drag = new QDrag(label);
            QMimeData *mime = new QMimeData;

            mime->setText(m_map[label]);   // 传组件类型
            drag->setMimeData(mime);

            drag->setPixmap(label->pixmap(Qt::ReturnByValue));
            drag->setHotSpot(me->pos());

            drag->exec(Qt::CopyAction);
            return true;
        }
    }

    return QObject::eventFilter(obj, event);
}
