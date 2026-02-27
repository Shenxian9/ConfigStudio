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

    // ⭐ 绑定 Slider 值变化，实时更新 Label
    connect(m_slider, &QwtSlider::valueChanged, this, [this](double v){
        m_valueLabel->setText(QString::number(v, 'f', 1));
        emit valueChanged(v);
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
        setValue(v.toDouble());
    }
}

void SliderComponent::setValue(double val)
{
    m_slider->setValue(val);
    m_valueLabel->setText(QString::number(val, 'f', 1));
    emit valueChanged(val);
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

