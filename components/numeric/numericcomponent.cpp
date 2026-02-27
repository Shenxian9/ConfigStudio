#include "numericcomponent.h"
#include <QPalette>
#include <QtMath>

NumericComponent::NumericComponent(QWidget *parent)
    : CanvasItem(parent)
{
    resize(200, 40);

    m_label = new QLabel(this);
    m_label->setAlignment(Qt::AlignCenter);
    m_label->setGeometry(rect());
    m_label->setStyleSheet("color:black;");

    updateText();
}

void NumericComponent::updateText()
{
    QString txt = QString::number(m_value, 'f', m_decimals);
    if (!m_unit.isEmpty())
        txt += " " + m_unit;

    m_label->setText(txt);
}

QVariantMap NumericComponent::properties() const
{
    QVariantMap map;
    map["value"]    = m_value;
    map["decimals"] = m_decimals;
    map["unit"]     = m_unit;
    map["fontSize"] = m_label->font().pointSize();
    map["bold"]     = m_label->font().bold();
    map["color"]    = m_label->palette().color(QPalette::WindowText).name();

    Qt::Alignment a = m_label->alignment();
    if (a & Qt::AlignLeft)        map["align"] = "left";
    else if (a & Qt::AlignRight)  map["align"] = "right";
    else                          map["align"] = "center";

    map["varId"] = m_varId;   // ⭐ 数据绑定ID

    return map;
}


void NumericComponent::setPropertyValue(const QString& key, const QVariant& v)
{
    if (key == "value") {
        m_value = v.toDouble();
        updateText();                 // ⭐ 运行态数据入口
    }
    else if (key == "decimals") {
        m_decimals = qMax(0, v.toInt());
        updateText();
    }
    else if (key == "unit") {
        m_unit = v.toString();
        updateText();
    }
    else if (key == "fontSize") {
        QFont f = m_label->font();
        f.setPointSize(v.toInt());
        m_label->setFont(f);
    }
    else if (key == "bold") {
        QFont f = m_label->font();
        f.setBold(v.toBool());
        m_label->setFont(f);
    }
    else if (key == "color") {
        QPalette p = m_label->palette();
        p.setColor(QPalette::WindowText, QColor(v.toString()));
        m_label->setPalette(p);
    }
    else if (key == "align") {
        if (v.toString() == "left")
            m_label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        else if (v.toString() == "right")
            m_label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        else
            m_label->setAlignment(Qt::AlignCenter);
    }
    else if (key == "varId") {
        QString newVarId = v.toString();

        if (newVarId == m_varId)
            return;   // ⭐ 防重复绑定

        // ⭐ 解绑旧变量
        if (!m_varId.isEmpty() && m_bindingMgr) {
            m_bindingMgr->unbind(m_varId, this, "value");
        }

        m_varId = newVarId;

        // ⭐ 绑定新变量
        if (m_bindingMgr && !m_varId.isEmpty()) {
            m_bindingMgr->bind(m_varId, this, "value");
            qDebug() << "Numeric bound to variable" << m_varId;
        }
    }
}


void NumericComponent::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    if (!m_label)
        return;

    m_label->setGeometry(rect());

    // === 和你 TextComponent 一致的自适应字体逻辑 ===
    QFont f = m_label->font();
    QString text = m_label->text();
    if (text.isEmpty()) return;

    int newPointSize = width() / (text.length() * 0.6);
    newPointSize = qMin(newPointSize, height());
    f.setPointSize(qMax(6, newPointSize));
    m_label->setFont(f);
}
