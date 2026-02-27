#include "histogramcomponent.h"
#include <qwt_plot.h>
#include <qwt_plot_histogram.h>
//#include <qwt_interval_data.h>
#include <qwt_scale_draw.h>
#include <qwt_text.h>      // 必须加


HistogramComponent::HistogramComponent(QWidget *parent)
    : CanvasItem(parent)
{
    resize(400, 300);

    m_plot = new QwtPlot(this);
    m_plot->setTitle("Histogram");
    m_plot->setAxisTitle(QwtPlot::xBottom, "X");
    m_plot->setAxisTitle(QwtPlot::yLeft, "Y");
    m_plot->setGeometry(rect());

    // 初始化柱状图
    m_histogram = new QwtPlotHistogram("Histogram");
    QVector<QwtIntervalSample> samples;
    samples.append(QwtIntervalSample(1, 0, 1));
    samples.append(QwtIntervalSample(4, 1, 2));
    samples.append(QwtIntervalSample(9, 2, 3));
    samples.append(QwtIntervalSample(16, 3, 4));
    m_histogram->setSamples(samples);
    m_histogram->attach(m_plot);
}

QVariantMap HistogramComponent::properties() const
{
    QVariantMap map;
    map["title"] = m_plot->title().text();
    map["xMin"] = m_plot->axisScaleDiv(QwtPlot::xBottom).lowerBound();
    map["xMax"] = m_plot->axisScaleDiv(QwtPlot::xBottom).upperBound();

    QVariantList values;
    for (const auto &s : m_samples)
        values.append(s.value);

    map["values"] = values;

    return map;
}

void HistogramComponent::setPropertyValue(const QString& key, const QVariant& v)
{
    if (key == "title") {
        m_plot->setTitle(v.toString());
    }
    else if (key == "values") {
        if (v.canConvert<QVariantList>()) {
            QVariantList list = v.toList();
            QVector<QwtIntervalSample> samples;
            for (int i = 0; i < list.size(); ++i) {
                double val = list[i].toDouble();
                samples.append(QwtIntervalSample(val, i, i+1));
            }
            m_samples = samples;                // 保存数据
            m_histogram->setSamples(m_samples); // 更新柱状图
            m_plot->replot();
        }
    }
}


void HistogramComponent::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    if (m_plot)
        m_plot->setGeometry(rect());
}
