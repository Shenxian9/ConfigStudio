#include "plotcomponent.h"
#include <qwt_text.h>
#include <qwt_scale_div.h>

#include <qwt_plot_curve.h>

#include <QHeaderView>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QFrame>
#include <QScroller>
#include <QRegularExpression>
#include <QtGlobal>
#include <QPen>

PlotComponent::PlotComponent(QWidget *parent)
    : CanvasItem(parent)
{
    resize(400, 300);

    m_plot = new QwtPlot(this);
    m_plot->setTitle("Plot");
    m_plot->setAxisTitle(QwtPlot::xBottom, "X");
    m_plot->setAxisTitle(QwtPlot::yLeft, "Y");
    m_plot->setGeometry(rect());
    m_plot->installEventFilter(this);

    m_varIds.clear();
    for (int i = 0; i < m_curveCount; ++i)
        m_varIds.append(QString());
    rebuildCurves();

}

QVariantMap PlotComponent::properties() const
{
    QVariantMap map;
    map["title"] = m_plot->title().text();
    map["xAxisTitle"] = m_plot->axisTitle(QwtPlot::xBottom).text();
    map["yAxisTitle"] = m_plot->axisTitle(QwtPlot::yLeft).text();
    const QVector<double> firstX = m_xSeries.value(0);
    map["xMin"] = firstX.isEmpty() ? 0.0 : firstX.first();
    map["xMax"] = firstX.isEmpty() ? 0.0 : firstX.last();
    map["curveCount"] = m_curveCount;
    map["maxPoints"] = m_maxPoints;
    if (m_curveCount > 0)
        map["varId"] = m_varIds.value(0);
    for (int i = 0; i < m_curveCount; ++i)
        map[QString("varId%1").arg(i + 1)] = m_varIds.value(i);
    return map;
}

void PlotComponent::setPropertyValue(const QString& key, const QVariant& v)
{
    if (key == "value") {
        appendValue(v.toDouble(), 0);
    }
    else if (key == "title") {
        m_plot->setTitle(v.toString());
    }
    else if (key == "xAxisTitle") {
        m_plot->setAxisTitle(QwtPlot::xBottom, v.toString());
        m_plot->replot();
    }
    else if (key == "yAxisTitle") {
        m_plot->setAxisTitle(QwtPlot::yLeft, v.toString());
        m_plot->replot();
    }
    else if (key == "curveCount") {
        int n = qMax(1, v.toInt());
        if (n == m_curveCount)
            return;

        // 先解绑旧绑定
        for (int i = 0; i < m_varIds.size(); ++i) {
            if (!m_varIds[i].isEmpty() && m_bindingMgr)
                m_bindingMgr->unbind(m_varIds[i], this, QString("value%1").arg(i + 1));
        }

        m_curveCount = n;
        while (m_varIds.size() < m_curveCount)
            m_varIds.append(QString());
        while (m_varIds.size() > m_curveCount)
            m_varIds.removeLast();

        rebuildCurves();
        rebindAllSeries();
    }
    else if (key == "maxPoints") {
        int mp = v.toInt();
        if (mp > 0)
            m_maxPoints = mp;
    }
    else if (key == "varId") {
        setPropertyValue("varId1", v);
    }
    else {
        QRegularExpression varIdRe("^varId(\\d+)$");
        QRegularExpressionMatch vm = varIdRe.match(key);
        if (vm.hasMatch()) {
            const int idx = vm.captured(1).toInt() - 1;
            if (idx < 0 || idx >= m_curveCount)
                return;

            const QString newVarId = v.toString();
            const QString oldVarId = m_varIds.value(idx);
            const QString propName = QString("value%1").arg(idx + 1);

            if (newVarId == oldVarId)
                return;

            if (!oldVarId.isEmpty() && m_bindingMgr)
                m_bindingMgr->unbind(oldVarId, this, propName);

            m_varIds[idx] = newVarId;
            if (!newVarId.isEmpty() && m_bindingMgr)
                m_bindingMgr->bind(newVarId, this, propName);
            return;
        }

        QRegularExpression valueRe("^value(\\d+)$");
        QRegularExpressionMatch m = valueRe.match(key);
        if (m.hasMatch()) {
            const int idx = m.captured(1).toInt() - 1;
            appendValue(v.toDouble(), idx);
            return;
        }
    }
}


void PlotComponent::resizeEvent(QResizeEvent* event)
{
    if (m_plot)
        m_plot->setGeometry(rect());
}

void PlotComponent::rebuildCurves()
{
    for (auto *c : m_curves) {
        if (!c) continue;
        c->detach();
        delete c;
    }
    m_curves.clear();

    if (m_xSeries.size() < m_curveCount)
        m_xSeries.resize(m_curveCount);
    else if (m_xSeries.size() > m_curveCount)
        m_xSeries = m_xSeries.mid(0, m_curveCount);

    if (m_ySeries.size() < m_curveCount)
        m_ySeries.resize(m_curveCount);
    else if (m_ySeries.size() > m_curveCount)
        m_ySeries = m_ySeries.mid(0, m_curveCount);

    static const QVector<QColor> palette = {
        QColor(255, 0, 0), QColor(0, 180, 0), QColor(0, 90, 255),
        QColor(255, 140, 0), QColor(160, 32, 240), QColor(0, 180, 180)
    };

    for (int i = 0; i < m_curveCount; ++i) {
        QwtPlotCurve *curve = new QwtPlotCurve(QString("Curve %1").arg(i + 1));
        curve->setRenderHint(QwtPlotItem::RenderAntialiased, true);
        curve->setPen(QPen(palette[i % palette.size()], 2));
        curve->setSamples(m_xSeries.value(i), m_ySeries.value(i));
        curve->attach(m_plot);
        m_curves.append(curve);
    }

    m_plot->replot();
}

void PlotComponent::rebindAllSeries()
{
    for (int i = 0; i < m_curveCount; ++i) {
        const QString varId = m_varIds.value(i);
        if (!varId.isEmpty() && m_bindingMgr)
            m_bindingMgr->bind(varId, this, QString("value%1").arg(i + 1));
    }
}


void PlotComponent::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        showHistoryDialog();
        event->accept();
        return;
    }
    CanvasItem::mouseDoubleClickEvent(event);
}

bool PlotComponent::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_plot && event->type() == QEvent::MouseButtonDblClick) {
        showHistoryDialog();
        return true;
    }
    return CanvasItem::eventFilter(watched, event);
}

void PlotComponent::showHistoryDialog()
{
    ensureHistoryPanel();
    refreshHistoryTable();

    if (m_historyPanel) {
        const int margin = 12;
        const int panelW = qMax(320, width() - margin * 2);
        const int panelH = qMax(220, height() - margin * 2);
        m_historyPanel->setGeometry((width() - panelW) / 2,
                                    (height() - panelH) / 2,
                                    panelW,
                                    panelH);
        m_historyPanel->show();
        m_historyPanel->raise();
    }
}

void PlotComponent::ensureHistoryPanel()
{
    if (m_historyPanel)
        return;

    QFrame *panel = new QFrame(this);
    panel->setFrameShape(QFrame::StyledPanel);
    panel->setStyleSheet("QFrame { background: white; border: 2px solid #7aa7d9; border-radius: 8px; }");

    QVBoxLayout *layout = new QVBoxLayout(panel);
    m_historyTable = new QTableWidget(panel);
    m_historyTable->setColumnCount(3);
    const QString xHeader = m_plot ? m_plot->axisTitle(QwtPlot::xBottom).text() : QString("X");
    const QString yHeader = m_plot ? m_plot->axisTitle(QwtPlot::yLeft).text() : QString("Y");
    m_historyTable->setHorizontalHeaderLabels({"#", xHeader, yHeader});
    m_historyTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_historyTable->verticalHeader()->setVisible(false);
    m_historyTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_historyTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_historyTable->setSelectionMode(QAbstractItemView::NoSelection);
    m_historyTable->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_historyTable->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);

    QFont tableFont = m_historyTable->font();
    tableFont.setPointSize(16);
    m_historyTable->setFont(tableFont);
    m_historyTable->verticalHeader()->setDefaultSectionSize(44);

    QScroller::grabGesture(m_historyTable->viewport(), QScroller::LeftMouseButtonGesture);
    layout->addWidget(m_historyTable);

    QPushButton *closeBtn = new QPushButton("Close", panel);
    closeBtn->setMinimumHeight(40);
    closeBtn->setFocusPolicy(Qt::NoFocus);
    layout->addWidget(closeBtn);
    QObject::connect(closeBtn, &QPushButton::clicked, panel, &QWidget::hide);

    panel->hide();
    m_historyPanel = panel;
}

void PlotComponent::refreshHistoryTable()
{
    if (!m_historyTable)
        return;

    const QString xHeader = m_plot ? m_plot->axisTitle(QwtPlot::xBottom).text() : QString("X");
    const QString yHeader = m_plot ? m_plot->axisTitle(QwtPlot::yLeft).text() : QString("Y");
    m_historyTable->setHorizontalHeaderLabels({"#", xHeader, yHeader});

    const QVector<double> xSeries = m_xSeries.value(0);
    const QVector<double> ySeries = m_ySeries.value(0);
    const int count = qMin(m_maxPoints, qMin(xSeries.size(), ySeries.size()));
    m_historyTable->setRowCount(count);

    const int start = xSeries.size() - count;
    for (int row = 0; row < count; ++row) {
        const int idx = start + row;
        m_historyTable->setItem(row, 0, new QTableWidgetItem(QString::number(row + 1)));
        const double y = (idx >= ySeries.size()) ? 0.0 : ySeries[idx];
        m_historyTable->setItem(row, 1, new QTableWidgetItem(QString::number(xSeries[idx], 'f', 2)));
        m_historyTable->setItem(row, 2, new QTableWidgetItem(QString::number(y, 'f', 2)));
    }
}

void PlotComponent::appendValue(double value, int seriesIndex)
{
    if (seriesIndex < 0 || seriesIndex >= m_curveCount)
        return;

    if (m_xSeries.size() < m_curveCount)
        m_xSeries.resize(m_curveCount);
    if (m_ySeries.size() < m_curveCount)
        m_ySeries.resize(m_curveCount);

    QVector<double> &x = m_xSeries[seriesIndex];
    QVector<double> &y = m_ySeries[seriesIndex];

    const double nextX = x.isEmpty() ? 0.0 : (x.last() + 1.0);
    x.append(nextX);
    y.append(value);

    while (x.size() > m_maxPoints) {
        x.remove(0);
        if (!y.isEmpty())
            y.remove(0);
    }

    if (m_curves.value(seriesIndex))
        m_curves[seriesIndex]->setSamples(x, y);

    double globalMinX = 0.0;
    double globalMaxX = 0.0;
    bool hasRange = false;
    for (const QVector<double> &xs : qAsConst(m_xSeries)) {
        if (xs.isEmpty())
            continue;
        if (!hasRange) {
            globalMinX = xs.first();
            globalMaxX = xs.last();
            hasRange = true;
        } else {
            globalMinX = qMin(globalMinX, xs.first());
            globalMaxX = qMax(globalMaxX, xs.last());
        }
    }

    if (hasRange)
        m_plot->setAxisScale(QwtPlot::xBottom, globalMinX, globalMaxX);

    m_plot->replot();
}
