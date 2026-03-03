#include "indicatorcomponent.h"

#include <QPainter>
#include <QtGlobal>
#include <QLoggingCategory>
#include <QDateTime>
#include "runtime/databindingmanager.h"


Q_LOGGING_CATEGORY(indicatorDiag, "configstudio.indicator")

namespace {
bool indicatorDiagEnabled()
{
    return qEnvironmentVariableIsSet("CONFIGSTUDIO_PROP_DIAG");
}

void indicatorLog(const QString &msg)
{
    if (!indicatorDiagEnabled()) return;
    qCInfo(indicatorDiag).noquote() << QDateTime::currentDateTime().toString("hh:mm:ss.zzz") << msg;
}
}

IndicatorComponent::IndicatorComponent(QWidget *parent)
    : CanvasItem(parent)
{
    resize(100, 100);

    m_title = new QLabel("LED", this);
    m_title->setAlignment(Qt::AlignCenter);

    m_valueLabel = new QLabel("0.0", this);
    m_valueLabel->setAlignment(Qt::AlignCenter);

    m_blinkTimer.setInterval(m_blinkIntervalMs);
    connect(&m_blinkTimer, &QTimer::timeout, this, [this]() {
        m_blinkState = !m_blinkState;
        update();
    });

    refreshBlinkState();
}

QVariantMap IndicatorComponent::properties() const
{
    QVariantMap map;
    map["title"] = m_title->text();
    map["on"] = m_on;
    map["blink"] = m_blink;
    map["mode"] = m_blinkMode;
    map["threshold"] = m_threshold;
    map["Interval/Ms"] = m_blinkIntervalMs;
    map["value"] = m_value;
    map["min"] = m_min;
    map["max"] = m_max;
    map["varId"] = m_varId;
    map["color"] = m_color.name();
    map["offColor"] = m_offColor.name();
    return map;
}

void IndicatorComponent::setPropertyValue(const QString& key, const QVariant& v)
{
    indicatorLog(QString("setPropertyValue key=%1 val=%2 this=%3")
                 .arg(key).arg(v.toString())
                 .arg(reinterpret_cast<quintptr>(this), 0, 16));
    if (key == "title") {
        m_title->setText(v.toString());
    }
    else if (key == "on") {
        m_on = v.toBool();
        refreshBlinkState();
    }
    else if (key == "blink") {
        m_blink = v.toBool();
        refreshBlinkState();
    }
    else if (key == "mode" || key == "blinkMode") {
        const QString mode = v.toString().trimmed().toLower();
        indicatorLog(QString("mode request=%1 current=%2").arg(mode).arg(m_blinkMode));
        if (mode == "above" || mode == "below") {
            m_blinkMode = mode;
            refreshBlinkState();
        }
    }
    else if (key == "threshold") {
        m_threshold = v.toDouble();
        refreshBlinkState();
    }
    else if (key == "Interval/Ms" || key == "blinkIntervalMs") {
        m_blinkIntervalMs = qMax(50, v.toInt());
        m_blinkTimer.setInterval(m_blinkIntervalMs);
        refreshBlinkState();
    }
    else if (key == "value") {
        setValue(v.toDouble());
    }
    else if (key == "min") {
        m_min = v.toDouble();
    }
    else if (key == "max") {
        m_max = v.toDouble();
    }
    else if (key == "varId") {
        QString newVarId = v.toString();

        if (newVarId == m_varId)
            return;

        if (!m_varId.isEmpty() && m_bindingMgr) {
            m_bindingMgr->unbind(m_varId, this, "value");
        }

        m_varId = newVarId;

        if (m_bindingMgr && !m_varId.isEmpty()) {
            m_bindingMgr->bind(m_varId, this, "value");
            qDebug() << "Indicator bound to variable" << m_varId;
        }
    }
    else if (key == "color" || key == "offColor") {
        const QString colorName = v.toString().trimmed().toLower();
        QColor target;
        if (colorName == "gray" || colorName == "grey") {
            target = QColor(128, 128, 128);
        } else if (colorName == "red") {
            target = QColor(255, 0, 0);
        } else if (colorName == "green") {
            target = QColor(0, 255, 0);
        } else if (colorName == "blue") {
            target = QColor(0, 0, 255);
        } else {
            target = QColor(v.toString());
        }

        if (target.isValid()) {
            if (key == "color")
                m_color = target;
            else
                m_offColor = target;
        }
    }

    update();
}

void IndicatorComponent::setValue(double value)
{
    m_value = qBound(m_min, value, m_max);
    m_valueLabel->setText(QString::number(m_value, 'f', 1));
    refreshBlinkState();
}

bool IndicatorComponent::isBlinkConditionMet() const
{
    if (m_blinkMode == "below")
        return m_value < m_threshold;

    // 默认按 above 处理，保证异常值时行为可预测。
    return m_value > m_threshold;
}

void IndicatorComponent::refreshBlinkState()
{
    const bool shouldBlink = m_on && m_blink && isBlinkConditionMet();

    if (shouldBlink) {
        if (!m_blinkTimer.isActive())
            m_blinkTimer.start();
    } else {
        m_blinkTimer.stop();
        m_blinkState = true;
    }

    update();
}

void IndicatorComponent::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    const int w = width();
    const int h = height();

    const int titleH = 22;
    const int valueH = 20;

    m_title->setGeometry(0, 0, w, titleH);
    m_valueLabel->setGeometry(0, h - valueH, w, valueH);
}

void IndicatorComponent::paintEvent(QPaintEvent *event)
{
    CanvasItem::paintEvent(event);

    const int w = width();
    const int h = height();

    const int titleH = 22;
    const int valueH = 20;
    QRect ledArea(0, titleH, w, h - titleH - valueH);

    const int size = qMin(ledArea.width(), ledArea.height()) * 0.65;
    const QPoint center = ledArea.center();

    QRect ledRect(center.x() - size / 2,
                  center.y() - size / 2,
                  size,
                  size);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const bool visible = m_on && (!m_blinkTimer.isActive() || m_blinkState);
    const QColor fill = visible ? m_color : m_offColor;

    p.setPen(Qt::NoPen);
    p.setBrush(fill.darker(150));
    p.drawEllipse(ledRect.adjusted(-2, -2, 2, 2));

    p.setBrush(fill);
    p.drawEllipse(ledRect);
}
