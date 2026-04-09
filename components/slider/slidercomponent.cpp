#include "runtime/databindingmanager.h"

#include <QEvent>
#include <QtMath>

#include "slidercomponent.h"

SliderComponent::SliderComponent(QWidget *parent)
    : CanvasItem(parent)
{
    resize(360, 150);

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
        if (m_step > 0.0) {
            const double snapped = qRound64(v / m_step) * m_step;
            if (!qFuzzyCompare(snapped + 1.0, v + 1.0) && !m_updatingFromBinding) {
                m_updatingFromBinding = true;
                m_slider->setValue(snapped);
                m_updatingFromBinding = false;
            }
            v = snapped;
        }
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
    map["step"]  = m_step;
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
    else if (key == "step") {
        const double nextStep = v.toDouble();
        m_step = nextStep <= 0.0 ? 1.0 : nextStep;
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
    if (m_step > 0.0)
        val = qRound64(val / m_step) * m_step;
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

    const int w = width();
    const int h = height();

    // 三段布局（标题 / slider / 数值）之间采用同一动态间距，随缩放自然拉伸。
    const int minSectionH = 20;
    const int gap = qBound(4, h / 22, 14);
    int titleH = qBound(minSectionH, h / 6, 34);
    int valueH = qBound(minSectionH, h / 6, 34);
    int sliderH = h - titleH - valueH - gap * 2;

    if (sliderH < 40) {
        const int lack = 40 - sliderH;
        const int cutTitle = qMin(titleH - minSectionH, (lack + 1) / 2);
        const int cutValue = qMin(valueH - minSectionH, lack / 2);
        titleH -= cutTitle;
        valueH -= cutValue;
        sliderH = qMax(40, h - titleH - valueH - gap * 2);
    }

    const int titleY = 0;
    const int sliderY = titleY + titleH + gap;
    const int valueY = sliderY + sliderH + gap;

    m_title->setGeometry(0, titleY, w, titleH);
    m_slider->setGeometry(0, sliderY, w, sliderH);
    m_valueLabel->setGeometry(0, valueY, w, qMax(minSectionH, h - valueY));
}
