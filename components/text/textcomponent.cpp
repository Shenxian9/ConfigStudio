#include "textcomponent.h"
#include <QPalette>
#include <QFontMetrics>

TextComponent::TextComponent(QWidget *parent)
    : CanvasItem(parent)
{
    resize(200, 40);

    m_label = new QLabel("Text", this);
    m_label->setAlignment(Qt::AlignCenter);
    m_label->setGeometry(rect());
    QPalette p = m_label->palette();
    p.setColor(QPalette::WindowText, QColor("black"));
    p.setColor(QPalette::Window, QColor("white"));
    m_label->setPalette(p);
    m_label->setAutoFillBackground(true);
}

QVariantMap TextComponent::properties() const
{
    QVariantMap map;
    map["text"]     = m_label->text();
    map["fontSize"] = m_label->font().pointSize();
    const QFont f = m_label->font();
    map["font"] = f.italic() ? "italic" : "normal";
    map["textColor"] = m_label->palette().color(QPalette::WindowText).name();
    map["blackBg"] = m_blackBg;

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
    else if (key == "font" || key == "bold") {
        QFont f = m_label->font();
        const QString style = v.toString().trimmed().toLower();
        f.setBold(false);
        if (style == "italic") {
            f.setItalic(true);
        } else {
            // 兼容旧 bool/bold 输入：统一降级为 normal，避免无效“bold”状态
            f.setItalic(false);
        }
        m_label->setFont(f);
    }
    else if (key == "textColor" || key == "textcolor" || key == "color") {
        QPalette p = m_label->palette();
        p.setColor(QPalette::WindowText, QColor(v.toString()));
        m_label->setPalette(p);
    }
    else if (key == "blackBg") {
        m_blackBg = v.toBool();
        QPalette p = m_label->palette();
        p.setColor(QPalette::Window, m_blackBg ? QColor("black") : QColor("white"));

        // 黑底时若文字仍是黑色，自动切到白色保证可读性；
        // 白底时若文字是白色，自动恢复黑色。
        const QColor textColor = p.color(QPalette::WindowText);
        if (m_blackBg && textColor == QColor("black"))
            p.setColor(QPalette::WindowText, QColor("white"));
        else if (!m_blackBg && textColor == QColor("white"))
            p.setColor(QPalette::WindowText, QColor("black"));

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
