#include "dialcomponent.h"
#include <qwt_dial_needle.h>
#include <QEvent>
#include <QPalette>

DialComponent::DialComponent(QWidget *parent)
    : CanvasItem(parent)
{
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

    applyDialPalette();
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

void DialComponent::changeEvent(QEvent *event)
{
    CanvasItem::changeEvent(event);

    if (!event)
        return;

    if (event->type() == QEvent::PaletteChange ||
        event->type() == QEvent::StyleChange ||
        event->type() == QEvent::ApplicationPaletteChange) {
        applyDialPalette();
    }
}

void DialComponent::applyDialPalette()
{
    if (!m_dial)
        return;

    // Canvas 主题是通过样式表切换的，不能仅依赖 palette 明暗判断。
    bool darkMode = false;
    QWidget *w = this;
    while (w) {
        if (QString::fromLatin1(w->metaObject()->className()) == "CanvasView") {
            const QString ss = w->styleSheet().toLower();
            darkMode = ss.contains("#505050");
            break;
        }
        w = w->parentWidget();
    }

    // 回退：若未拿到 CanvasView 样式，再用调色板明暗兜底。
    if (!w) {
        const QPalette canvasPalette = palette();
        darkMode = canvasPalette.color(QPalette::Window).lightness() < 128;
    }

    QPalette dialPalette = m_dial->palette();

    // 盘面颜色：浅色模式灰色，深色模式白色。
    const QColor faceColor = darkMode ? QColor("#ffffff") : QColor("#bfbfbf");
    // 刻度与数字颜色。
    const QColor scaleColor = darkMode ? QColor("#101010") : QColor("#1f1f1f");

    // QwtDial 关键角色：
    // Base       -> 框内背景
    // WindowText -> 刻度圈内部圆盘填充
    // Text       -> 刻度和数字
    const QPalette::ColorGroup groups[] = {
        QPalette::Active,
        QPalette::Inactive,
        QPalette::Disabled
    };

    for (QPalette::ColorGroup group : groups) {
        dialPalette.setColor(group, QPalette::Base, faceColor);
        dialPalette.setColor(group, QPalette::Window, faceColor);
        dialPalette.setColor(group, QPalette::Button, faceColor);

        // 关键：WindowText 控制内盘填充，不用于刻度文字。
        dialPalette.setColor(group, QPalette::WindowText, faceColor);

        // 刻度/数字颜色使用 Text。
        dialPalette.setColor(group, QPalette::Text, scaleColor);
        dialPalette.setColor(group, QPalette::ButtonText, scaleColor);
        dialPalette.setColor(group, QPalette::Foreground, scaleColor);
    }

    m_dial->setAutoFillBackground(false);
    m_dial->setBackgroundRole(QPalette::Base);
    m_dial->setForegroundRole(QPalette::Text);
    m_dial->setStyleSheet(QString());
    m_dial->setPalette(dialPalette);
    m_dial->update();
}

