#include "runtime/databindingmanager.h"

#include <QEvent>

#include "wheelcomponent.h"

WheelComponent::WheelComponent(QWidget *parent)
    : CanvasItem(parent)
{
    resize(80, 200);

    m_title = new QLabel("Wheel", this);
    m_title->setAlignment(Qt::AlignCenter);

    m_value = new QLabel("0", this);
    m_value->setAlignment(Qt::AlignCenter);

    m_wheel = new QwtWheel(this);
    m_wheel->setOrientation(Qt::Vertical);
    m_wheel->setRange(0.0, 100.0);
    m_wheel->setValue(50.0);
    m_wheel->setSingleStep(1.0);
    // 触控设备上过大的 mass 会让拖动显得“发涩”，降低后跟手更好。
    m_wheel->setMass(0.08);
    m_wheel->installEventFilter(this);

    // ⭐ 核心：wheel 变化 → label 更新
    connect(m_wheel, &QwtWheel::valueChanged,
            this, [this](double v){
                m_value->setText(QString::number(v, 'f', 1));

                if (!m_updatingFromBinding && m_bindingMgr && !m_varId.isEmpty())
                    m_bindingMgr->publishValue(m_varId, v);
            });

    // 初始化显示
    m_value->setText(QString::number(m_wheel->value(), 'f', 1));
}


QVariantMap WheelComponent::properties() const
{
    QVariantMap map;
    map["title"] = m_title->text();
    map["min"]   = m_wheel->minimum();
    map["max"]   = m_wheel->maximum();
    map["value"] = m_wheel->value();
    map["step"]  = m_wheel->singleStep();
    map["varId"] = m_varId;
    return map;
}

void WheelComponent::setPropertyValue(const QString& key, const QVariant& v)
{
    if (key == "title") {
        m_title->setText(v.toString());
    }
    else if (key == "min") {
        m_wheel->setRange(v.toDouble(), m_wheel->maximum());
    }
    else if (key == "max") {
        m_wheel->setRange(m_wheel->minimum(), v.toDouble());
    }
    else if (key == "value") {
        if (m_userInteracting)
            return;

        const double val = v.toDouble();
        m_updatingFromBinding = true;
        m_wheel->setValue(val);
        m_updatingFromBinding = false;
        m_value->setText(QString::number(val, 'f', 1)); // ⭐ 同步
    }
    else if (key == "step") {
        m_wheel->setSingleStep(v.toDouble());
    }
    else if (key == "varId") {
        const QString newVarId = v.toString().trimmed();
        if (newVarId == m_varId)
            return;

        if (!m_varId.isEmpty() && m_bindingMgr)
            m_bindingMgr->unbind(m_varId, this, "value");

        m_varId = newVarId;

        if (!m_varId.isEmpty() && m_bindingMgr)
            m_bindingMgr->bind(m_varId, this, "value");
    }
}



bool WheelComponent::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_wheel && event) {
        switch (event->type()) {
        case QEvent::MouseButtonPress:
        case QEvent::TouchBegin:
            m_userInteracting = true;
            break;
        case QEvent::MouseButtonRelease:
        case QEvent::TouchEnd:
        case QEvent::Leave:
            m_userInteracting = false;
            break;
        default:
            break;
        }
    }

    return CanvasItem::eventFilter(watched, event);
}

void WheelComponent::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    int w = width();
    int h = height();

    int titleH = 24;
    int valueH = 20;

    m_title->setGeometry(0, 0, w, titleH);
    m_value->setGeometry(0, h - valueH, w, valueH);

    // 标题与数值标签之间整块区域都作为可拖动区域，提升触控连续性。
    const int marginX = qMax(4, w / 16);
    const int marginY = 4;
    const int wheelY = titleH + marginY;
    const int wheelH = qMax(24, h - titleH - valueH - marginY * 2);
    const int wheelW = qMax(24, w - marginX * 2);

    m_wheel->setGeometry(marginX, wheelY, wheelW, wheelH);

    // 轮体厚度按当前控件宽度自适应，避免太窄导致难以拖动。
    const int wheelVisualWidth = qMax(16, qMin(wheelW - 8, 80));
    m_wheel->setWheelWidth(wheelVisualWidth);
}
