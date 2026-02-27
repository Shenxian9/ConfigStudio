#include "indicatorcomponent.h"
#include <QPainter>

IndicatorComponent::IndicatorComponent(QWidget *parent)
    : CanvasItem(parent)
{
    resize(80, 80);

    m_title = new QLabel("LED", this);
    m_title->setAlignment(Qt::AlignCenter);

    // 闪烁定时器
    m_blinkTimer.setInterval(500);
    connect(&m_blinkTimer, &QTimer::timeout, this, [this]() {
        m_blinkState = !m_blinkState;
        update();
    });
}

QVariantMap IndicatorComponent::properties() const
{
    QVariantMap map;
    map["title"] = m_title->text();
    map["on"]    = m_on;
    map["blink"] = m_blink;
    map["color"] = m_color.name();
    return map;
}

void IndicatorComponent::setPropertyValue(const QString& key, const QVariant& v)
{
    if (key == "title") {
        m_title->setText(v.toString());
    }
    else if (key == "on") {
        m_on = v.toBool();
        update();
    }
    else if (key == "blink") {
        m_blink = v.toBool();
        if (m_blink)
            m_blinkTimer.start();
        else {
            m_blinkTimer.stop();
            m_blinkState = true;
        }
        update();
    }
    else if (key == "color") {
        m_color = QColor(v.toString());
        update();
    }
}

void IndicatorComponent::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    int w = width();
    int h = height();

    m_title->setGeometry(0, 0, w, h / 2);
}

void IndicatorComponent::paintEvent(QPaintEvent *event)
{
    CanvasItem::paintEvent(event);

    int w = width();
    int h = height();

    QRect ledArea(0, h / 2, w, h / 2);

    int size = qMin(ledArea.width(), ledArea.height()) * 0.6;
    QPoint center = ledArea.center();

    QRect ledRect(
        center.x() - size / 2,
        center.y() - size / 2,
        size,
        size
        );

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    bool visible = m_on && (!m_blink || m_blinkState);

    QColor fill = visible ? m_color : QColor(80, 80, 80);

    // 外圈
    p.setPen(Qt::NoPen);
    p.setBrush(fill.darker(150));
    p.drawEllipse(ledRect.adjusted(-2, -2, 2, 2));

    // 内圈
    p.setBrush(fill);
    p.drawEllipse(ledRect);
}
