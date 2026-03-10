#include "textcomponent.h"
#include <QPalette>

TextComponent::TextComponent(QWidget *parent)
    : CanvasItem(parent)
{
    resize(200, 40);

    m_label = new QLabel("Text", this);
    m_label->setAlignment(Qt::AlignCenter);
    m_label->setGeometry(rect());
    QPalette p = m_label->palette();
    p.setColor(QPalette::WindowText, QColor("black"));
    m_label->setPalette(p);
}

QVariantMap TextComponent::properties() const
{
    QVariantMap map;
    map["text"]     = m_label->text();
    map["fontSize"] = m_label->font().pointSize();
    const QFont f = m_label->font();
    if (f.italic())
        map["bold"] = "italic";
    else if (f.bold())
        map["bold"] = "bold";
    else
        map["bold"] = "normal";
    map["textColor"] = m_label->palette().color(QPalette::WindowText).name();

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
        f.setPointSize(qMax(1, v.toInt()));
        m_label->setFont(f);
    }
    else if (key == "bold") {
        QFont f = m_label->font();
        const QString style = v.toString().trimmed().toLower();
        if (style == "bold") {
            f.setBold(true);
            f.setItalic(false);
        } else if (style == "italic") {
            f.setBold(false);
            f.setItalic(true);
        } else {
            // 兼容旧 bool：true->bold, false->normal
            const bool isBold = (style == "true" || style == "1") || v.toBool();
            f.setBold(isBold);
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
}

void TextComponent::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    if (!m_label)
        return;

    m_label->setGeometry(rect());
}
