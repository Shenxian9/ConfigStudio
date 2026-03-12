#ifndef PLOTCOMPONENT_H
#define PLOTCOMPONENT_H

#pragma once
#include "canvas/canvasitem.h"
#include "canvas/canvasview.h"
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <QTableWidget>
#include <QStringList>
#include <QLabel>
#include <QTimer>

class PlotComponent : public CanvasItem {
    Q_OBJECT
public:
    QVariantMap properties() const override;
    void setPropertyValue(const QString&, const QVariant&) override;

    explicit PlotComponent(QWidget *parent = nullptr);

    QString type() const override { return "plot"; }

    void resizeEvent(QResizeEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
    void appendValue(double value, int seriesIndex = 0);

private:
    QwtPlot *m_plot;
    QVector<QwtPlotCurve*> m_curves;
    QVector<double> m_xData;
    QVector<QVector<double>> m_ySeries;
    QStringList m_varIds;   // 每条曲线绑定一个变量ID

    int m_curveCount = 1;
    int m_maxPoints = 50; // 最多显示点数
    int m_refreshRate = 5; // Hz
    double m_yMin = 0.0;
    double m_yMax = 100.0;

    QTimer m_sampleTimer;
    QVector<double> m_latestValues;
    QVector<bool> m_hasLatestValues;

    QWidget *m_historyPanel = nullptr;
    QTableWidget *m_historyTable = nullptr;
    QWidget *m_bindingConflictPanel = nullptr;
    QLabel *m_bindingConflictLabel = nullptr;

    void showHistoryDialog();
    void ensureHistoryPanel();
    void refreshHistoryTable();
    void ensureBindingConflictPanel();
    void showBindingConflict(const QString &message);
    void rebuildCurves();
    void rebindAllSeries();
    void applySampleTimer();
    void sampleAtFixedRate();
};


#endif // PLOTCOMPONENT_H
