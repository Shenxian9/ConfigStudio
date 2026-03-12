#include "iconlabel.h"
#include <QSizePolicy>

IconLabel::IconLabel(QWidget* parent)
    : QLabel(parent)
{
    setAlignment(Qt::AlignCenter);

    // 避免 pixmap 的 sizeHint 反向撑大网格布局，导致底部 frame 抬高。
    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    setMinimumSize(0, 0);
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


QSize IconLabel::sizeHint() const
{
    // 防止 QLabel 根据 pixmap/text 的 hint 撑高网格行。
    return QSize(1, 1);
}

QSize IconLabel::minimumSizeHint() const
{
    return QSize(1, 1);
}
