#ifndef PLOTCOMPONENT_H
#define PLOTCOMPONENT_H

#pragma once
#include "canvas/canvasitem.h"
#include "canvas/canvasview.h"
#include <qwt_plot.h>
#include <qwt_plot_curve.h>

class PlotComponent : public CanvasItem {
    Q_OBJECT
public:
    QVariantMap properties() const override;
    void setPropertyValue(const QString&, const QVariant&) override;

    explicit PlotComponent(QWidget *parent = nullptr);

    QString type() const override { return "plot"; }

    void resizeEvent(QResizeEvent* event) override;
    void appendValue(double value);

private:
    QwtPlot *m_plot;
    QwtPlotCurve *m_curve;
    QVector<double> m_xData;
    QVector<double> m_yData;
    QString m_varId;   // 当前绑定的变量ID


    int m_maxPoints = 50; // 最多显示100个点
};


#endif // PLOTCOMPONENT_H
