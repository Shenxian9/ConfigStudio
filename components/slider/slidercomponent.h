#ifndef SLIDERCOMPONENT_H
#define SLIDERCOMPONENT_H

#pragma once
#include "canvas/canvasitem.h"
#include <qwt_slider.h>
#include <qwt_scale_draw.h>
#include <QLabel>

class SliderComponent : public CanvasItem {
    Q_OBJECT
public:
    explicit SliderComponent(QWidget *parent = nullptr);

    QVariantMap properties() const override;
    void setPropertyValue(const QString&, const QVariant&) override;

    QString type() const override { return "slider"; }

    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
    void setValue(double val);

signals:
    void valueChanged(double val);

private:
    QwtSlider *m_slider;
    QLabel      *m_valueLabel;
    QLabel    *m_title;

    QString m_varId;
    double m_step = 1.0;
    bool m_updatingFromBinding = false;
    bool m_userInteracting = false;
    bool m_hasPendingUserValue = false;
    double m_pendingUserValue = 0.0;
};

#endif
