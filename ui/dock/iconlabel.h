#ifndef ICONLABEL_H
#define ICONLABEL_H

#include <QLabel>
#include <QPixmap>
#include <QPainter>


class IconLabel : public QLabel
{
    Q_OBJECT
public:
    explicit IconLabel(QWidget* parent = nullptr);

    void setIcon(const QString& resourcePath);
    void updatePixmap();

    void setDesignSize(const QSize& size) { m_designSize = size; updatePixmap(); }
protected:
    void resizeEvent(QResizeEvent* event) override;
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

private:


    QString m_iconPath;
    QPixmap m_originalPixmap;

    QSize m_designSize;
};

#endif
