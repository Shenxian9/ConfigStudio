#include "textcomponent.h"
#include <QPalette>

TextComponent::TextComponent(QWidget *parent)
    : CanvasItem(parent)
{
    resize(200, 40);

    m_label = new QLabel("Text", this);
    m_label->setAlignment(Qt::AlignCenter);
    m_label->setGeometry(rect());
    m_label->setStyleSheet("color:black;");
}

QVariantMap TextComponent::properties() const
{
    QVariantMap map;
    map["text"]     = m_label->text();
    map["fontSize"] = m_label->font().pointSize();
    map["bold"]     = m_label->font().bold();
    map["color"]    = m_label->palette().color(QPalette::WindowText).name();

    Qt::Alignment a = m_label->alignment();
    if (a & Qt::AlignLeft)   map["align"] = "left";
    else if (a & Qt::AlignRight) map["align"] = "right";
    else map["align"] = "center";

    return map;
}

void TextComponent::setPropertyValue(const QString& key, const QVariant& v)
{
    if (key == "text") {
        m_label->setText(v.toString());
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
}

void TextComponent::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    if (!m_label)
        return;

    m_label->setGeometry(rect());

    QFont f = m_label->font();
    QString text = m_label->text();
    if (text.isEmpty()) return;

    // 粗略估计：每个字符占字体宽度约0.6 * pointSize
    int newPointSize = width() / (text.length() * 0.6);
    newPointSize = qMin(newPointSize, height()); // 不超过组件高度
    f.setPointSize(newPointSize);
    m_label->setFont(f);
}

