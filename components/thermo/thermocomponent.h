#ifndef THERMOCOMPONENT_H
#define THERMOCOMPONENT_H

#pragma once
#include "canvas/canvasitem.h"
#include "canvas/canvasview.h"
#include <qwt_thermo.h>
#include <qwt_scale_draw.h>
#include <QLabel>
#include <qwt_scale_div.h>

class ThermoComponent : public CanvasItem {
    Q_OBJECT
public:
    explicit ThermoComponent(QWidget *parent = nullptr);

    QVariantMap properties() const override;
    void setPropertyValue(const QString&, const QVariant&) override;

    QString type() const override { return "thermo"; }

    void resizeEvent(QResizeEvent *event) override;

    void setValue(double val);
signals:
    void valueChanged(double val);

private:
    QwtThermo *m_thermo;
    QLabel    *m_title;
    QLabel      *m_value;

    QString m_varId;
};

#endif
