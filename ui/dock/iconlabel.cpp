#include "iconlabel.h"

IconLabel::IconLabel(QWidget* parent)
    : QLabel(parent)
{
    setAlignment(Qt::AlignCenter);
}

void IconLabel::setIcon(const QString& resourcePath)
{
    m_iconPath = resourcePath;
    m_originalPixmap = QPixmap(resourcePath);

    // 记录设计时大小，作为缩放基准
    if (!m_designSize.isValid())
        m_designSize = size(); // 只有第一次设置时记录

    updatePixmap();
}


void IconLabel::resizeEvent(QResizeEvent* event)
{
    QLabel::resizeEvent(event);
    updatePixmap();
}

void IconLabel::updatePixmap()
{
    if (m_originalPixmap.isNull() || size().isEmpty())
        return;

    // 以当前可用区域为上限进行缩放，避免图标放大后破坏布局比例。
    constexpr int kIconPadding = 6;
    const QSize availableSize = contentsRect().adjusted(
                                    kIconPadding / 2,
                                    kIconPadding / 2,
                                    -kIconPadding / 2,
                                    -kIconPadding / 2)
                                    .size()
                                    .expandedTo(QSize(1, 1));

    const QPixmap scaled = m_originalPixmap.scaled(
        availableSize,
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation);

    setPixmap(scaled);
}


