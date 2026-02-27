#ifndef INDICATORCOMPONENT_H
#define INDICATORCOMPONENT_H

#pragma once
#include "canvas/canvasitem.h"
#include <QLabel>
#include <QTimer>

class IndicatorComponent : public CanvasItem {
    Q_OBJECT
public:
    explicit IndicatorComponent(QWidget *parent = nullptr);

    QVariantMap properties() const override;
    void setPropertyValue(const QString&, const QVariant&) override;

    QString type() const override { return "indicator"; }

    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    QLabel *m_title;

    bool   m_on   = true;
    bool   m_blink = true;
    QColor m_color = Qt::green;

    QTimer m_blinkTimer;
    bool   m_blinkState = true;
};

#endif
