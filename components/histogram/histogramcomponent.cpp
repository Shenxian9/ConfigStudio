#include "histogramcomponent.h"

#include <qwt_plot.h>
#include <qwt_scale_div.h>

#include <QPen>
#include <QBrush>
#include <QRegularExpression>
#include <QtGlobal>

HistogramComponent::HistogramComponent(QWidget *parent)
    : CanvasItem(parent)
{
    resize(400, 300);

    m_plot = new QwtPlot(this);
    m_plot->setTitle("Histogram");
    m_plot->setAxisTitle(QwtPlot::xBottom, "X");
    m_plot->setAxisTitle(QwtPlot::yLeft, "Y");
    m_plot->setGeometry(rect());

    m_values = QVector<double>(m_curveCount, 0.0);
    m_varIds = QStringList(m_curveCount, QString());

    rebuildBars();
    refreshSamples();
}

QVariantMap HistogramComponent::properties() const
{
    QVariantMap map;
    map["title"] = m_plot->title().text();
    map["xAxisTitle"] = m_plot->axisTitle(QwtPlot::xBottom).text();
    map["yAxisTitle"] = m_plot->axisTitle(QwtPlot::yLeft).text();
    map["xMin"] = m_plot->axisScaleDiv(QwtPlot::xBottom).lowerBound();
    map["xMax"] = m_plot->axisScaleDiv(QwtPlot::xBottom).upperBound();
    map["curveCount"] = m_curveCount;
    map["barCount"] = m_curveCount;

    if (m_curveCount > 0) {
        map["value"] = m_values.value(0);
        map["varId"] = m_varIds.value(0);
    }

    QVariantList values;
    for (int i = 0; i < m_curveCount; ++i) {
        map[QString("value%1").arg(i + 1)] = m_values.value(i);
        map[QString("varId%1").arg(i + 1)] = m_varIds.value(i);
        values.append(m_values.value(i));
    }
    map["values"] = values;

    return map;
}

void HistogramComponent::setPropertyValue(const QString& key, const QVariant& v)
{
    if (key == "title") {
        m_plot->setTitle(v.toString());
    }
    else if (key == "xAxisTitle") {
        m_plot->setAxisTitle(QwtPlot::xBottom, v.toString());
    }
    else if (key == "yAxisTitle") {
        m_plot->setAxisTitle(QwtPlot::yLeft, v.toString());
    }
    else if (key == "curveCount" || key == "barCount") {
        const int n = qMax(1, v.toInt());
        if (n == m_curveCount)
            return;

        for (int i = 0; i < m_varIds.size(); ++i) {
            if (!m_varIds[i].isEmpty() && m_bindingMgr)
                m_bindingMgr->unbind(m_varIds[i], this, QString("value%1").arg(i + 1));
        }

        m_curveCount = n;
        m_values.resize(m_curveCount);
        while (m_varIds.size() < m_curveCount)
            m_varIds.append(QString());
        while (m_varIds.size() > m_curveCount)
            m_varIds.removeLast();

        rebuildBars();
        refreshSamples();
        rebindAllSeries();
    }
    else if (key == "values") {
        if (v.canConvert<QVariantList>()) {
            const QVariantList list = v.toList();
            const int count = qMin(list.size(), m_curveCount);
            for (int i = 0; i < count; ++i)
                m_values[i] = list[i].toDouble();
            refreshSamples();
        }
    }
    else if (key == "value") {
        setPropertyValue("value1", v);
    }
    else if (key == "varId") {
        setPropertyValue("varId1", v);
    }
    else {
        QRegularExpression valueRe("^value(\\d+)$");
        QRegularExpressionMatch vm = valueRe.match(key);
        if (vm.hasMatch()) {
            const int idx = vm.captured(1).toInt() - 1;
            if (idx < 0 || idx >= m_curveCount)
                return;
            m_values[idx] = v.toDouble();
            refreshSamples();
            return;
        }

        QRegularExpression varIdRe("^varId(\\d+)$");
        QRegularExpressionMatch idm = varIdRe.match(key);
        if (idm.hasMatch()) {
            const int idx = idm.captured(1).toInt() - 1;
            if (idx < 0 || idx >= m_curveCount)
                return;

            const QString oldVarId = m_varIds.value(idx);
            const QString newVarId = v.toString().trimmed();
            if (oldVarId == newVarId)
                return;

            if (!newVarId.isEmpty()) {
                for (int i = 0; i < m_varIds.size(); ++i) {
                    if (i != idx && m_varIds.value(i) == newVarId)
                        return;
                }
            }

            const QString propName = QString("value%1").arg(idx + 1);
            if (!oldVarId.isEmpty() && m_bindingMgr)
                m_bindingMgr->unbind(oldVarId, this, propName);

            m_varIds[idx] = newVarId;
            if (!newVarId.isEmpty() && m_bindingMgr)
                m_bindingMgr->bind(newVarId, this, propName);
            return;
        }
    }

    m_plot->replot();
}

void HistogramComponent::rebuildBars()
{
    for (QwtPlotHistogram *bar : m_bars) {
        if (!bar)
            continue;
        bar->detach();
        delete bar;
    }
    m_bars.clear();

    static const QVector<QColor> palette = {
        QColor(255, 0, 0), QColor(0, 180, 0), QColor(0, 90, 255),
        QColor(255, 140, 0), QColor(160, 32, 240), QColor(0, 180, 180)
    };

    for (int i = 0; i < m_curveCount; ++i) {
        auto *bar = new QwtPlotHistogram(QString("Bar %1").arg(i + 1));
        const QColor color = palette[i % palette.size()];
        bar->setBrush(QBrush(color));
        bar->setPen(QPen(color.darker(130), 1));
        bar->attach(m_plot);
        m_bars.append(bar);
    }
}

void HistogramComponent::rebindAllSeries()
{
    for (int i = 0; i < m_curveCount; ++i) {
        const QString varId = m_varIds.value(i);
        if (!varId.isEmpty() && m_bindingMgr)
            m_bindingMgr->bind(varId, this, QString("value%1").arg(i + 1));
    }
}

void HistogramComponent::refreshSamples()
{
    if (m_values.size() < m_curveCount)
        m_values.resize(m_curveCount);

    for (int i = 0; i < m_bars.size(); ++i) {
        if (!m_bars[i])
            continue;

        const double left = static_cast<double>(i);
        const double right = static_cast<double>(i + 1);
        QVector<QwtIntervalSample> samples;
        samples.append(QwtIntervalSample(m_values.value(i), left, right));
        m_bars[i]->setSamples(samples);
    }

    m_plot->setAxisScale(QwtPlot::xBottom, 0.0, qMax(1, m_curveCount));
    m_plot->replot();
}

void HistogramComponent::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    if (m_plot)
        m_plot->setGeometry(rect());
}
