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

    // 计算缩放比例：相对于设计尺寸
    double wRatio = double(width()) / m_designSize.width();
    double hRatio = double(height()) / m_designSize.height();
    double scale = qMin(wRatio, hRatio);

    QSize targetSize = m_designSize * scale*1.9;

    QPixmap scaled = m_originalPixmap.scaled(
        targetSize,
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation
        );

    setPixmap(scaled);
}


