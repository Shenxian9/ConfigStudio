#include "switchcomponent.h"

SwitchComponent::SwitchComponent(QWidget *parent)
    : CanvasItem(parent)
{
    resize(120, 60);

    m_title = new QLabel("Switch", this);
    m_title->setAlignment(Qt::AlignCenter);

    m_switch = new QCheckBox(this);
    m_switch->setText("");     // 不显示文字
    m_switch->setChecked(false);

    // 简单样式（可后期统一）
    m_switch->setStyleSheet(R"(
        QCheckBox::indicator {
            width: 40px;
            height: 20px;
        }
    )");
}

QVariantMap SwitchComponent::properties() const
{
    QVariantMap map;
    map["title"]   = m_title->text();
    map["checked"] = m_switch->isChecked();
    return map;
}

void SwitchComponent::setPropertyValue(const QString& key, const QVariant& v)
{
    if (key == "title") {
        m_title->setText(v.toString());
    }
    else if (key == "checked") {
        m_switch->setChecked(v.toBool());
    }
}

void SwitchComponent::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    int w = width();
    int h = height();

    m_title->setGeometry(0, 0, w, h / 2);

    int switchHeight = h / 2;
    int indicatorH = switchHeight * 0.6;
    int indicatorW = indicatorH * 2;   // 开关一般是 2:1

    m_switch->setGeometry(
        w/2 - indicatorW/2,
        h/2 + (switchHeight - indicatorH)/2,
        indicatorW,
        indicatorH
        );

    // ⭐ 关键：动态设置 indicator 大小
    m_switch->setStyleSheet(QString(R"(
        QCheckBox::indicator {
            width: %1px;
            height: %2px;
        }
    )").arg(indicatorW).arg(indicatorH));
}
