#include "numericcomponent.h"
#include <QPalette>
#include <QFontMetrics>

NumericComponent::NumericComponent(QWidget *parent)
    : CanvasItem(parent)
{
    resize(200, 40);

    m_label = new QLabel(this);
    m_label->setAlignment(Qt::AlignCenter);
    m_label->setGeometry(rect());
    QPalette p = m_label->palette();
    p.setColor(QPalette::WindowText, QColor("black"));
    m_label->setPalette(p);

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
    const QFont f = m_label->font();
    map["font"] = f.italic() ? "italic" : "normal";
    map["textColor"] = m_label->palette().color(QPalette::WindowText).name();

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
        f.setPointSize(qMax(1, v.toInt()));
        m_label->setFont(f);
    }
    else if (key == "font" || key == "bold") {
        QFont f = m_label->font();
        const QString style = v.toString().trimmed().toLower();
        f.setBold(false);
        if (style == "italic") {
            f.setItalic(true);
        } else {
            // 兼容旧 bool/bold 输入：统一按 normal 处理
            f.setItalic(false);
        }
        m_label->setFont(f);
    }
    else if (key == "textColor" || key == "textcolor" || key == "color") {
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

    const int availW = qMax(1, width() - 8);
    const int availH = qMax(1, height() - 8);

    QFont f = m_label->font();
    int pointSize = qMax(1, int(availH * 0.72));
    f.setPointSize(pointSize);

    const QString text = m_label->text().isEmpty() ? QStringLiteral(" ") : m_label->text();
    while (pointSize > 1) {
        QFontMetrics fm(f);
        if (fm.horizontalAdvance(text) <= availW && fm.height() <= availH)
            break;
        --pointSize;
        f.setPointSize(pointSize);
    }

    m_label->setFont(f);
}
