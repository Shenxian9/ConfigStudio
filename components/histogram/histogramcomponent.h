#ifndef HISTOGRAMCOMPONENT_H
#define HISTOGRAMCOMPONENT_H

#pragma once
#include "canvas/canvasitem.h"
#include <qwt_plot.h>
#include <qwt_plot_histogram.h>
#include <QStringList>

class HistogramComponent : public CanvasItem {
    Q_OBJECT
public:
    explicit HistogramComponent(QWidget *parent = nullptr);

    QVariantMap properties() const override;
    void setPropertyValue(const QString&, const QVariant&) override;

    QString type() const override { return "histogram"; }

    void resizeEvent(QResizeEvent* event) override;

private:
    void rebuildBars();
    void rebindAllSeries();
    void refreshSamples();

    QwtPlot *m_plot;
    QVector<QwtPlotHistogram*> m_bars;
    QVector<double> m_values;
    QStringList m_varIds;

    int m_barCount = 1;

    double m_yMin = 0.0;
    double m_yMax = 100.0;
};

#endif
