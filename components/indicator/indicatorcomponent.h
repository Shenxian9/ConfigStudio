#ifndef INDICATORCOMPONENT_H
#define INDICATORCOMPONENT_H

#pragma once
#include "canvas/canvasitem.h"
#include <QColor>
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
    void setValue(double value);
    void refreshBlinkState();

private:
    QLabel *m_title;
    QLabel *m_valueLabel;

    bool   m_on = true;
    bool   m_blink = true;
    bool   m_blinkWhenAbove = true;
    bool   m_blinkState = true;

    double m_value = 0.0;
    double m_min = 0.0;
    double m_max = 100.0;
    double m_threshold = 50.0;

    int    m_blinkIntervalMs = 500;

    QColor m_color = Qt::green;
    QColor m_offColor = QColor(80, 80, 80);

    QString m_varId;

    QTimer m_blinkTimer;
};

#endif
