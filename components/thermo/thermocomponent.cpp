#include "thermocomponent.h"

ThermoComponent::ThermoComponent(QWidget *parent)
    : CanvasItem(parent)
{
    resize(120, 300);

    // 顶部标题
    m_title = new QLabel("Thermo", this);
    m_title->setAlignment(Qt::AlignCenter);

    // 温度值显示
    m_value = new QLabel("0", this);
    m_value->setAlignment(Qt::AlignCenter);

    // 温度柱
    m_thermo = new QwtThermo(this);
    m_thermo->setOrientation(Qt::Vertical);
    m_thermo->setScale(0, 100);

    m_thermo->setFillBrush(Qt::red);

    // 初始化显示
    m_value->setText(QString::number(m_thermo->value(), 'f', 1));
}

QVariantMap ThermoComponent::properties() const
{
    QVariantMap map;

    QwtScaleDiv div = m_thermo->scaleDiv();

    map["title"] = m_title->text();
    map["value"] = m_thermo->value();   // 仅展示
    map["min"] = div.lowerBound();
    map["max"] = div.upperBound();
    map["varId"] = m_varId;             // ⭐ 绑定变量ID

    return map;
}


void ThermoComponent::setPropertyValue(const QString& key, const QVariant& v)
{
    QwtScaleDiv div = m_thermo->scaleDiv();
    double min = div.lowerBound();
    double max = div.upperBound();

    if (key == "title") {
        m_title->setText(v.toString());
    }
    else if (key == "min") {
        m_thermo->setScale(v.toDouble(), max);
    }
    else if (key == "max") {
        m_thermo->setScale(min, v.toDouble());
    }
    else if (key == "value") {
        setValue(v.toDouble());   // ⭐ 来自数据绑定的实时推送
    }
    else if (key == "varId") {
        QString newVarId = v.toString();

        if (newVarId == m_varId)
            return;   // 避免重复绑定

        // ⭐ 先解绑旧变量
        if (!m_varId.isEmpty() && m_bindingMgr) {
            m_bindingMgr->unbind(m_varId, this, "value");
        }

        m_varId = newVarId;

        // ⭐ 绑定新变量
        if (m_bindingMgr && !m_varId.isEmpty()) {
            m_bindingMgr->bind(m_varId, this, "value");
            qDebug() << "Thermo bound to variable" << m_varId;
        }
    }
}


void ThermoComponent::setValue(double val)
{
    m_thermo->setValue(val);
    m_value->setText(QString::number(val, 'f', 1));
    emit valueChanged(val);
}

void ThermoComponent::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    int w = width();
    int h = height();

    int titleH = 20;
    int valueH = 20;

    m_title->setGeometry(0, 0, w, titleH);
    m_value->setGeometry(0, titleH, w, valueH);

    int thermoY = titleH + valueH;
    int thermoH = h - thermoY;


    int totalWidth = width();
    int thermoWidth = qMin(totalWidth * 7/8, 400);  // 柱子视觉宽度
    int x = (totalWidth - thermoWidth) / 2-thermoWidth/2+20;         // 居中

    m_thermo->setGeometry(x, titleH + valueH + 4, thermoWidth, thermoH - 8);
    m_thermo->setContentsMargins(0,0,0,0); // 去掉内部边距
}
