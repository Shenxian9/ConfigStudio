#include "dialcomponent.h"
#include <qwt_dial_needle.h>

DialComponent::DialComponent(QWidget *parent)
    : CanvasItem(parent)
{
    qDebug()<<"?";
    resize(200, 220);

    m_title = new QLabel("Dial", this);
    m_title->setAlignment(Qt::AlignCenter);
    m_title->setGeometry(0, 0, width(), 20);

    m_dial = new QwtDial(this);
    m_dial->setGeometry(0, 20, width(), height() - 20);

    m_dial->setScale(0, 100);
    m_dial->setValue(30);
    m_dial->setWrapping(false);
    m_dial->setReadOnly(true);

    m_dial->setNeedle(new QwtDialSimpleNeedle(
        QwtDialSimpleNeedle::Arrow, true, Qt::red));
}


QVariantMap DialComponent::properties() const
{
    QVariantMap map;

    const QwtRoundScaleDraw *draw =
        dynamic_cast<const QwtRoundScaleDraw*>(m_dial->scaleDraw());

    double min = 0.0, max = 100.0;
    if (draw) {
        min = draw->scaleDiv().lowerBound();
        max = draw->scaleDiv().upperBound();
    }

    map["title"] = m_title->text();
    map["min"]   = min;
    map["max"]   = max;
    map["value"] = m_dial->value();  // 展示值
    map["varId"] = m_varId;          // ⭐ 数据绑定ID

    return map;
}


void DialComponent::setPropertyValue(const QString& key, const QVariant& v)
{
    const QwtRoundScaleDraw *draw =
        dynamic_cast<const QwtRoundScaleDraw*>(m_dial->scaleDraw());

    double min = 0.0, max = 100.0;
    if (draw) {
        min = draw->scaleDiv().lowerBound();
        max = draw->scaleDiv().upperBound();
    }

    if (key == "title") {
        m_title->setText(v.toString());
    }
    else if (key == "min") {
        m_dial->setScale(v.toDouble(), max);
    }
    else if (key == "max") {
        m_dial->setScale(min, v.toDouble());
    }
    else if (key == "value") {
        m_dial->setValue(v.toDouble());   // ⭐ 运行态数据推送
    }
    else if (key == "varId") {
        QString newVarId = v.toString();

        if (newVarId == m_varId)
            return;   // 防止重复绑定

        // ⭐ 解绑旧变量
        if (!m_varId.isEmpty() && m_bindingMgr) {
            m_bindingMgr->unbind(m_varId, this, "value");
        }

        m_varId = newVarId;

        // ⭐ 绑定新变量
        if (m_bindingMgr && !m_varId.isEmpty()) {
            m_bindingMgr->bind(m_varId, this, "value");
            qDebug() << "Dial bound to variable" << m_varId;
        }
    }
}


void DialComponent::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);  // 保持基类行为

    if (m_title)
        m_title->setGeometry(0, 0, width(), 20); // 顶部标题固定高度

    if (m_dial)
        m_dial->setGeometry(0, 20, width(), height() - 20); // 剩余部分给表盘
}


