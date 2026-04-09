#include "switchcomponent.h"
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>

SwitchVisual::SwitchVisual(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_OpaquePaintEvent, false);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void SwitchVisual::setChecked(bool checked)
{
    if (m_checked == checked)
        return;
    m_checked = checked;
    update();
    emit toggled(m_checked);
}

void SwitchVisual::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::TextAntialiasing, true);
    p.setRenderHint(QPainter::SmoothPixmapTransform, true);

    const QRectF outer = rect().adjusted(1, 1, -1, -1);
    if (outer.width() < 4 || outer.height() < 4)
        return;

    const qreal base = qMax<qreal>(1.0, qMin(outer.width(), outer.height()));
    const qreal margin = qBound<qreal>(base * 0.10, 2.0, base * 0.18);
    const qreal trackH = qMax<qreal>(base * 0.56, outer.height() - margin * 2.0);
    const qreal trackW = qMax<qreal>(trackH * 1.8, outer.width() - margin * 2.0);
    const QRectF track((outer.width() - trackW) / 2.0,
                       (outer.height() - trackH) / 2.0,
                       trackW, trackH);
    const qreal radius = track.height() / 2.0;

    const QColor offFill("#f4f4f4");
    const QColor offBorder("#c9c9c9");
    const QColor onFill("#34c759");
    const QColor onBorder("#2ea94b");
    p.setPen(QPen(m_checked ? onBorder : offBorder, qMax<qreal>(1.0, trackH * 0.05)));
    p.setBrush(m_checked ? onFill : offFill);
    p.drawRoundedRect(track, radius, radius);

    const qreal thumbInset = qMax<qreal>(1.0, trackH * 0.08);
    const qreal thumbD = qMax<qreal>(8.0, trackH - 2.0 * thumbInset);
    const qreal thumbY = track.center().y() - thumbD / 2.0;
    const qreal thumbX = m_checked
                             ? (track.right() - thumbInset - thumbD)
                             : (track.left() + thumbInset);
    const QRectF thumb(thumbX, thumbY, thumbD, thumbD);

    QColor shadow(0, 0, 0, 45);
    p.setPen(Qt::NoPen);
    p.setBrush(shadow);
    p.drawEllipse(thumb.translated(0, qMax<qreal>(1.0, thumbD * 0.06)));

    p.setBrush(Qt::white);
    p.setPen(QPen(QColor(220, 220, 220), qMax<qreal>(0.8, thumbD * 0.03)));
    p.drawEllipse(thumb);

    QFont f = font();
    f.setBold(true);
    f.setPointSizeF(qBound<qreal>(8.0, trackH * 0.30, trackH * 0.40));
    p.setFont(f);
    const QString text = m_checked ? QStringLiteral("ON") : QStringLiteral("OFF");
    p.setPen(m_checked ? QColor("#ffffff") : QColor("#8b8b8b"));

    const qreal textPadding = qMax<qreal>(4.0, trackH * 0.18);
    QRectF textRect = track.adjusted(textPadding, 0, -textPadding, 0);
    if (m_checked) {
        textRect.setRight(thumb.left() - textPadding);
    } else {
        textRect.setLeft(thumb.right() + textPadding);
    }
    if (textRect.width() > 4.0)
        p.drawText(textRect, Qt::AlignVCenter | Qt::AlignHCenter, text);
}

void SwitchVisual::mouseReleaseEvent(QMouseEvent *event)
{
    if (event && event->button() == Qt::LeftButton) {
        setChecked(!m_checked);
        event->accept();
        return;
    }
    QWidget::mouseReleaseEvent(event);
}

SwitchComponent::SwitchComponent(QWidget *parent)
    : CanvasItem(parent)
{
    resize(120, 60);

    m_title = new QLabel("Switch", this);
    m_title->setAlignment(Qt::AlignCenter);

    m_switch = new SwitchVisual(this);
    m_switch->setChecked(false);
}

QVariantMap SwitchComponent::properties() const
{
    QVariantMap map;
    map["title"]   = m_title->text();
    map["checked"] = m_switch->isChecked();
    return map;
}

void SwitchComponent::setPropertyValue(const QString& key, const QVariant& v)
{
    if (key == "title") {
        m_title->setText(v.toString());
    }
    else if (key == "checked") {
        m_switch->setChecked(v.toBool());
    }
}

void SwitchComponent::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    const int w = width();
    const int h = height();

    const int titleH = qBound(18, h * 2 / 5, qMax(18, h - 24));
    m_title->setGeometry(0, 0, w, titleH);

    const int switchTop = titleH + qMax(2, h / 30);
    const int switchH = qMax(20, h - switchTop - qMax(2, h / 30));
    const int switchW = qMax(36, w - qMax(2, w / 20) * 2);
    m_switch->setGeometry((w - switchW) / 2, switchTop, switchW, switchH);
}
