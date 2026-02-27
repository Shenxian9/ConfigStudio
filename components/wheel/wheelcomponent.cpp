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
    m_wheel->setMass(0.3);

    // ⭐ 核心：wheel 变化 → label 更新
    connect(m_wheel, &QwtWheel::valueChanged,
            this, [this](double v){
                m_value->setText(QString::number(v, 'f', 1));
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
        double val = v.toDouble();
        m_wheel->setValue(val);
        m_value->setText(QString::number(val, 'f', 1)); // ⭐ 同步
    }
    else if (key == "step") {
        m_wheel->setSingleStep(v.toDouble());
    }
}


void WheelComponent::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    int w = width();
    int h = height();

    int titleH = 24;
    int valueH = 20;

    m_title->setGeometry(0, 0, w, titleH);
    m_value->setGeometry(0, h-titleH, w, valueH);

    // 剩余区域
    int wheelAreaY = titleH + valueH;
    int wheelAreaH = h - wheelAreaY;

    // ⭐ 控制视觉宽度（不要太胖）
    int wheelVisualWidth = qMin(w / 6*4, 400);   // 最大 40
    int wheelAreaWidth   = wheelVisualWidth + 20; // 留点边距

    int x = (w - wheelAreaWidth) / 2;

    m_wheel->setGeometry(
        x,
        wheelAreaY - 12,                // 上下留白
        wheelAreaWidth,
        wheelAreaH - 16
        );

    // ⭐ 控制滚轮“厚度”
    m_wheel->setWheelWidth(wheelVisualWidth);
}

