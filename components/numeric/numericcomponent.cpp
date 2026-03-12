#include "numericcomponent.h"
#include <QPalette>
#include <QFontMetrics>
#include <QEvent>

static QString textColorNameForProperty(const QColor &c)
{
    if (c == QColor("black")) return "black";
    if (c == QColor("white")) return "white";
    if (c == QColor("red")) return "red";
    if (c == QColor("green")) return "green";
    if (c == QColor("blue")) return "blue";
    if (c == QColor("yellow")) return "yellow";
    if (c == QColor("gray") || c == QColor("grey")) return "gray";
    return c.name();
}

NumericComponent::NumericComponent(QWidget *parent)
    : CanvasItem(parent)
{
    resize(200, 40);

    m_label = new QLabel(this);
    m_label->setAlignment(Qt::AlignCenter);
    m_label->setGeometry(rect());
    QPalette p = m_label->palette();
    p.setColor(QPalette::WindowText, QColor("black"));
    p.setColor(QPalette::Window, QColor("white"));
    m_label->setPalette(p);
    m_label->setAutoFillBackground(true);

    m_themeDark = isCanvasDarkMode();
    m_blackBg = m_themeDark;
    applyLabelStyle();

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
    map["textColor"] = textColorNameForProperty(m_label->palette().color(QPalette::WindowText));
    map["blackBg"] = m_blackBg;

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
            f.setItalic(false);
        }
        m_label->setFont(f);
    }
    else if (key == "textColor" || key == "textcolor" || key == "color") {
        QPalette p = m_label->palette();
        QColor c(v.toString());
        if (!c.isValid())
            c = QColor("black");
        p.setColor(QPalette::WindowText, c);
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

void NumericComponent::changeEvent(QEvent *event)
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

bool NumericComponent::isCanvasDarkMode() const
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

void NumericComponent::applyLabelStyle()
{
    if (!m_label)
        return;

    QPalette p = m_label->palette();
    const QColor textColor = p.color(QPalette::WindowText);
    const QColor bgColor = m_blackBg ? QColor("black") : QColor("white");

    p.setColor(QPalette::Window, bgColor);
    m_label->setPalette(p);

    // 显式样式覆盖 CanvasView QLabel 通用规则，确保文字颜色可由属性表修改。
    m_label->setStyleSheet(QString("QLabel { color: %1; background-color: %2; }")
                           .arg(textColor.name(), bgColor.name()));
}
