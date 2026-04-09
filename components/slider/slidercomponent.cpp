#include "runtime/databindingmanager.h"

#include <QEvent>

#include "slidercomponent.h"

SliderComponent::SliderComponent(QWidget *parent)
    : CanvasItem(parent)
{
    resize(300, 120);

    // 顶部标题
    m_title = new QLabel("Slider", this);
    m_title->setAlignment(Qt::AlignCenter);

    // 当前值显示 Label
    m_valueLabel = new QLabel("50", this);
    m_valueLabel->setAlignment(Qt::AlignCenter);

    // Slider
    m_slider = new QwtSlider(this);
    m_slider->setOrientation(Qt::Horizontal);
    m_slider->setScale(0, 100);
    m_slider->setValue(50);
    m_slider->installEventFilter(this);

    // ⭐ 绑定 Slider 值变化，实时更新 Label
    connect(m_slider, &QwtSlider::valueChanged, this, [this](double v){
        m_valueLabel->setText(QString::number(v, 'f', 1));
        emit valueChanged(v);

        if (m_updatingFromBinding || !m_bindingMgr || m_varId.isEmpty())
            return;
        if (m_userInteracting) {
            m_pendingUserValue = v;
            m_hasPendingUserValue = true;
            return;
        }
        m_bindingMgr->publishValue(m_varId, v);
    });

    // 初始化显示
    m_valueLabel->setText(QString::number(m_slider->value(), 'f', 1));
}

QVariantMap SliderComponent::properties() const
{
    QVariantMap map;
    map["title"] = m_title->text();
    map["min"]   = m_slider->lowerBound();
    map["max"]   = m_slider->upperBound();
    map["value"] = m_slider->value();
    map["varId"] = m_varId;
    return map;
}

void SliderComponent::setPropertyValue(const QString& key, const QVariant& v)
{
    double min = m_slider->lowerBound();
    double max = m_slider->upperBound();

    if (key == "title") {
        m_title->setText(v.toString());
    }
    else if (key == "min") {
        m_slider->setScale(v.toDouble(), max);
    }
    else if (key == "max") {
        m_slider->setScale(min, v.toDouble());
    }
    else if (key == "value") {
        if (m_userInteracting)
            return;
        setValue(v.toDouble());
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

void SliderComponent::setValue(double val)
{
    m_updatingFromBinding = true;
    m_slider->setValue(val);
    m_updatingFromBinding = false;

    m_valueLabel->setText(QString::number(val, 'f', 1));
}


bool SliderComponent::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_slider && event) {
        switch (event->type()) {
        case QEvent::MouseButtonPress:
        case QEvent::TouchBegin:
            m_userInteracting = true;
            m_hasPendingUserValue = false;
            break;
        case QEvent::MouseButtonRelease:
        case QEvent::TouchEnd:
        case QEvent::Leave:
            m_userInteracting = false;
            if (m_hasPendingUserValue && m_bindingMgr && !m_varId.isEmpty()) {
                m_bindingMgr->publishValue(m_varId, m_pendingUserValue);
                m_hasPendingUserValue = false;
            }
            break;
        default:
            break;
        }
    }

    return CanvasItem::eventFilter(watched, event);
}

void SliderComponent::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    int w = width();
    int h = height();

    int titleH = 20;      // 标题高度
    int valueH = 20;      // 数值显示高度
    int sliderH = h - titleH - valueH-30; // Slider 剩余高度

    m_title->setGeometry(0, 0, w, titleH);
    m_valueLabel->setGeometry(0, h-valueH, w, valueH);

    // ⭐ Slider 保持原有高度，刻度不会被遮挡
    m_slider->setGeometry(0, titleH +15, w, sliderH);
}
