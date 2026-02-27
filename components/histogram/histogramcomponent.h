#ifndef HISTOGRAMCOMPONENT_H
#define HISTOGRAMCOMPONENT_H

#pragma once
#include "canvas/canvasitem.h"
#include <qwt_plot.h>
#include <qwt_plot_histogram.h>


class HistogramComponent : public CanvasItem {
    Q_OBJECT
public:
    explicit HistogramComponent(QWidget *parent = nullptr);

    QVariantMap properties() const override;
    void setPropertyValue(const QString&, const QVariant&) override;

    QString type() const override { return "histogram"; }

    void resizeEvent(QResizeEvent* event) override;

private:
    QwtPlot *m_plot;
    QwtPlotHistogram *m_histogram;
    QVector<QwtIntervalSample> m_samples; // 保存柱子数据
};

#endif
