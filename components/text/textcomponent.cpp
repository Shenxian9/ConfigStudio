#include "textcomponent.h"
#include <QPalette>
#include <QFontMetrics>
#include <QEvent>

static QString textColorNameForProperty(const QColor &c)
{
    if (c == QColor("black")) return "black";
    if (c == QColor("red")) return "red";
    if (c == QColor("green")) return "green";
    if (c == QColor("blue")) return "blue";
    if (c == QColor("yellow")) return "yellow";
    if (c == QColor("gray") || c == QColor("grey")) return "gray";
    return c.name();
}

TextComponent::TextComponent(QWidget *parent)
    : CanvasItem(parent)
{
    resize(200, 40);

    m_label = new QLabel("Text", this);
    m_label->setAlignment(Qt::AlignCenter);
    m_label->setGeometry(rect());

    QPalette p = m_label->palette();
    m_textColor = QColor("black");
    p.setColor(QPalette::WindowText, m_textColor);
    p.setColor(QPalette::Window, QColor("white"));
    m_label->setPalette(p);
    m_label->setAutoFillBackground(true);

    m_themeDark = isCanvasDarkMode();
    m_blackBg = m_themeDark;
    applyLabelStyle();
}

QVariantMap TextComponent::properties() const
{
    QVariantMap map;
    map["text"]     = m_label->text();
    map["fontSize"] = m_label->font().pointSize();
    const QFont f = m_label->font();
    map["font"] = f.italic() ? "italic" : "normal";
    map["textColor"] = textColorNameForProperty(m_textColor);
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
            f.setItalic(false);
        }
        m_label->setFont(f);
    }
    else if (key == "textColor" || key == "textcolor" || key == "color") {
        QPalette p = m_label->palette();
        QColor c(v.toString());
        if (!c.isValid())
            c = QColor("black");
        m_textColor = c;
        p.setColor(QPalette::WindowText, m_textColor);
        m_label->setPalette(p);
        applyLabelStyle();
    }
    else if (key == "blackBg") {
        m_blackBg = v.toBool();
        applyLabelStyle();
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

void TextComponent::changeEvent(QEvent *event)
{
    CanvasItem::changeEvent(event);

    if (!event)
        return;

    if (event->type() == QEvent::PaletteChange ||
        event->type() == QEvent::StyleChange ||
        event->type() == QEvent::ApplicationPaletteChange) {
        const bool dark = isCanvasDarkMode();
        m_themeDark = dark;
        m_blackBg = dark;
        applyLabelStyle();
    }
}

bool TextComponent::isCanvasDarkMode() const
{
    const QWidget *w = this;
    while (w) {
        if (QString::fromLatin1(w->metaObject()->className()) == "CanvasView") {
            const QString ss = w->styleSheet().toLower();
            return ss.contains("#505050");
        }
        w = w->parentWidget();
    }

    const QPalette canvasPalette = palette();
    return canvasPalette.color(QPalette::Window).lightness() < 128;
}

void TextComponent::applyLabelStyle()
{
    if (!m_label)
        return;

    QPalette p = m_label->palette();
    const QColor textColor = m_textColor;
    const QColor bgColor = m_blackBg ? QColor("black") : QColor("white");

    p.setColor(QPalette::Window, bgColor);
    p.setColor(QPalette::WindowText, textColor);
    m_label->setPalette(p);

    // blackBg=true 且属性色为 black 时，显示白字保证可读；但属性值保持 black。
    const QColor displayTextColor = (m_blackBg && textColor == QColor("black"))
            ? QColor("white")
            : textColor;

    // 显式样式覆盖 CanvasView QLabel 通用规则，确保文字颜色可由属性表修改。
    m_label->setStyleSheet(QString("QLabel { color: %1; background-color: %2; }")
                           .arg(displayTextColor.name(), bgColor.name()));
}
