#include "plotcomponent.h"
#include <qwt_text.h>
#include <qwt_scale_div.h>
#include <qwt_scale_draw.h>

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


namespace {
class PlotIntegerScaleDraw : public QwtScaleDraw {
public:
    QwtText label(double value) const override
    {
        return QwtText(QString::number(qRound(value)));
    }
};
}

PlotComponent::PlotComponent(QWidget *parent)
    : CanvasItem(parent)
{
    resize(400, 300);

    m_plot = new QwtPlot(this);
    m_plot->setTitle("Plot");
    m_plot->setAxisTitle(QwtPlot::xBottom, "X");
    m_plot->setAxisTitle(QwtPlot::yLeft, "Y");
    m_plot->setAxisScaleDraw(QwtPlot::xBottom, new PlotIntegerScaleDraw());
    m_plot->setAxisMaxMinor(QwtPlot::xBottom, 0);
    m_plot->setAxisMaxMajor(QwtPlot::xBottom, 6);
    m_plot->setAxisAutoScale(QwtPlot::yLeft, false);
    m_plot->setAxisScale(QwtPlot::yLeft, m_yMin, m_yMax);
    m_plot->setGeometry(rect());
    m_plot->installEventFilter(this);

    m_varIds.clear();
    for (int i = 0; i < m_curveCount; ++i)
        m_varIds.append(QString());
    rebuildCurves();

    m_xData.reserve(m_maxPoints);

    m_latestValues = QVector<double>(m_curveCount, 0.0);
    m_hasLatestValues = QVector<bool>(m_curveCount, false);

    connect(&m_sampleTimer, &QTimer::timeout, this, [this]() { sampleAtFixedRate(); });
    applySampleTimer();
}


QVariantMap PlotComponent::properties() const
{
    QVariantMap map;
    map["title"] = m_plot->title().text();
    map["xAxisTitle"] = m_plot->axisTitle(QwtPlot::xBottom).text();
    map["yAxisTitle"] = m_plot->axisTitle(QwtPlot::yLeft).text();
    map["xMin"] = m_xData.isEmpty() ? 0.0 : m_xData.first();
    map["xMax"] = m_xData.isEmpty() ? 0.0 : m_xData.last();
    map["yMin"] = m_yMin;
    map["yMax"] = m_yMax;
    map["curveCount"] = m_curveCount;
    map["maxPoints"] = m_maxPoints;
    map["refreshRate"] = m_refreshRate;
    for (int i = 0; i < m_curveCount; ++i)
        map[QString("varId%1").arg(i + 1)] = m_varIds.value(i);
    return map;
}

void PlotComponent::setPropertyValue(const QString& key, const QVariant& v)
{
    if (key == "value") {
        if (!m_latestValues.isEmpty()) {
            m_latestValues[0] = v.toDouble();
            m_hasLatestValues[0] = true;
        }
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
    else if (key == "yMin") {
        m_yMin = v.toDouble();
        if (m_yMax <= m_yMin)
            m_yMax = m_yMin + 1.0;
        m_plot->setAxisAutoScale(QwtPlot::yLeft, false);
        m_plot->setAxisScale(QwtPlot::yLeft, m_yMin, m_yMax);
        m_plot->replot();
    }
    else if (key == "yMax") {
        m_yMax = v.toDouble();
        if (m_yMax <= m_yMin)
            m_yMin = m_yMax - 1.0;
        m_plot->setAxisAutoScale(QwtPlot::yLeft, false);
        m_plot->setAxisScale(QwtPlot::yLeft, m_yMin, m_yMax);
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

        m_latestValues.resize(m_curveCount);
        m_hasLatestValues.resize(m_curveCount);
        for (int i = 0; i < m_hasLatestValues.size(); ++i) {
            if (!m_hasLatestValues[i])
                m_latestValues[i] = 0.0;
        }

        rebuildCurves();
        rebindAllSeries();
    }
    else if (key == "maxPoints") {
        int mp = v.toInt();
        if (mp > 0)
            m_maxPoints = mp;
    }
    else if (key == "refreshRate") {
        const int hz = qMax(1, v.toInt());
        if (hz != m_refreshRate) {
            m_refreshRate = hz;
            applySampleTimer();
        }
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

            const QString newVarId = v.toString().trimmed();
            const QString oldVarId = m_varIds.value(idx);
            const QString propName = QString("value%1").arg(idx + 1);

            if (newVarId == oldVarId)
                return;

            if (!newVarId.isEmpty()) {
                for (int i = 0; i < m_varIds.size(); ++i) {
                    if (i != idx && m_varIds.value(i) == newVarId) {
                        showBindingConflict(
                            tr("Variable ID '%1' is already bound to curve %2. Please choose a different varId.")
                                .arg(newVarId)
                                .arg(i + 1));
                        return;
                    }
                }
            }

            if (!oldVarId.isEmpty() && m_bindingMgr)
                m_bindingMgr->unbind(oldVarId, this, propName);

            m_varIds[idx] = newVarId;
            if (!newVarId.isEmpty() && m_bindingMgr) {
                m_bindingMgr->bind(newVarId, this, propName);

                QVariant currentVal;
                if (m_bindingMgr->currentValue(newVarId, &currentVal)) {
                    m_latestValues[idx] = currentVal.toDouble();
                    m_hasLatestValues[idx] = true;
                }
            }
            return;
        }

        QRegularExpression valueRe("^value(\\d+)$");
        QRegularExpressionMatch m = valueRe.match(key);
        if (m.hasMatch()) {
            const int idx = m.captured(1).toInt() - 1;
            if (idx < 0 || idx >= m_curveCount)
                return;
            m_latestValues[idx] = v.toDouble();
            m_hasLatestValues[idx] = true;
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

    if (m_ySeries.size() < m_curveCount)
        m_ySeries.resize(m_curveCount);
    else if (m_ySeries.size() > m_curveCount)
        m_ySeries = m_ySeries.mid(0, m_curveCount);

    for (int i = 0; i < m_ySeries.size(); ++i) {
        while (m_ySeries[i].size() < m_xData.size())
            m_ySeries[i].append(qQNaN());
        while (m_ySeries[i].size() > m_xData.size())
            m_ySeries[i].remove(0);
    }

    static const QVector<QColor> palette = {
        QColor(255, 0, 0), QColor(0, 180, 0), QColor(0, 90, 255),
        QColor(255, 140, 0), QColor(160, 32, 240), QColor(0, 180, 180)
    };

    for (int i = 0; i < m_curveCount; ++i) {
        QwtPlotCurve *curve = new QwtPlotCurve(QString("Curve %1").arg(i + 1));
        curve->setRenderHint(QwtPlotItem::RenderAntialiased, true);
        curve->setPen(QPen(palette[i % palette.size()], 2));
        curve->setSamples(m_xData, m_ySeries.value(i));
        curve->attach(m_plot);
        m_curves.append(curve);
    }

    m_plot->setAxisAutoScale(QwtPlot::yLeft, false);
    m_plot->setAxisScale(QwtPlot::yLeft, m_yMin, m_yMax);
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



void PlotComponent::applySampleTimer()
{
    const int hz = qMax(1, m_refreshRate);
    const int intervalMs = qMax(1, int(1000 / hz));
    m_sampleTimer.start(intervalMs);
}

void PlotComponent::sampleAtFixedRate()
{
    for (int i = 0; i < m_curveCount; ++i) {
        if (!m_hasLatestValues.value(i))
            continue;
        appendValue(m_latestValues.value(i), i);
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
        auto enablePanelInteraction = [](QWidget *root) {
            if (!root) return;
            root->setAttribute(Qt::WA_TransparentForMouseEvents, false);
            const auto children = root->findChildren<QWidget*>();
            for (QWidget *w : children) {
                if (w)
                    w->setAttribute(Qt::WA_TransparentForMouseEvents, false);
            }
        };
        enablePanelInteraction(m_historyPanel);

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

    const QVector<double> ySeries = m_ySeries.value(0);
    const int count = qMin(m_maxPoints, qMin(m_xData.size(), ySeries.size()));
    m_historyTable->setRowCount(count);

    const int start = m_xData.size() - count;
    for (int row = 0; row < count; ++row) {
        const int idx = start + row;
        m_historyTable->setItem(row, 0, new QTableWidgetItem(QString::number(row + 1)));
        const double y = (idx >= ySeries.size()) ? 0.0 : ySeries[idx];
        m_historyTable->setItem(row, 1, new QTableWidgetItem(QString::number(m_xData[idx], 'f', 2)));
        m_historyTable->setItem(row, 2, new QTableWidgetItem(QString::number(y, 'f', 2)));
    }
}

void PlotComponent::ensureBindingConflictPanel()
{
    if (m_bindingConflictPanel)
        return;

    QFrame *panel = new QFrame(this);
    panel->setFrameShape(QFrame::StyledPanel);
    panel->setStyleSheet("QFrame { background: white; border: 2px solid #d97a7a; border-radius: 8px; }");

    QVBoxLayout *layout = new QVBoxLayout(panel);
    QLabel *title = new QLabel(tr("Binding Conflict"), panel);
    QFont titleFont = title->font();
    titleFont.setBold(true);
    titleFont.setPointSize(14);
    title->setFont(titleFont);
    layout->addWidget(title);

    m_bindingConflictLabel = new QLabel(panel);
    m_bindingConflictLabel->setWordWrap(true);
    QFont msgFont = m_bindingConflictLabel->font();
    msgFont.setPointSize(12);
    m_bindingConflictLabel->setFont(msgFont);
    layout->addWidget(m_bindingConflictLabel);

    QPushButton *okBtn = new QPushButton("OK", panel);
    okBtn->setMinimumHeight(38);
    okBtn->setFocusPolicy(Qt::NoFocus);
    layout->addWidget(okBtn);
    QObject::connect(okBtn, &QPushButton::clicked, panel, &QWidget::hide);

    panel->hide();
    m_bindingConflictPanel = panel;
}

void PlotComponent::showBindingConflict(const QString &message)
{
    ensureBindingConflictPanel();
    if (!m_bindingConflictPanel || !m_bindingConflictLabel)
        return;

    m_bindingConflictPanel->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    const auto children = m_bindingConflictPanel->findChildren<QWidget*>();
    for (QWidget *w : children) {
        if (w)
            w->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    }

    m_bindingConflictLabel->setText(message);

    const int margin = 14;
    const int panelW = qMax(320, width() - margin * 2);
    const int panelH = 160;
    m_bindingConflictPanel->setGeometry((width() - panelW) / 2,
                                        qMax(10, (height() - panelH) / 2 - height() / 8),
                                        panelW,
                                        panelH);
    m_bindingConflictPanel->show();
    m_bindingConflictPanel->raise();
}

void PlotComponent::appendValue(double value, int seriesIndex)
{
    if (seriesIndex < 0 || seriesIndex >= m_curveCount)
        return;

    if (m_ySeries.size() < m_curveCount)
        m_ySeries.resize(m_curveCount);

    auto appendNewXPoint = [this]() {
        const double nextX = m_xData.isEmpty() ? 0.0 : (m_xData.last() + 1.0);
        m_xData.append(nextX);
        for (int i = 0; i < m_ySeries.size(); ++i)
            m_ySeries[i].append(qQNaN());
    };

    if (m_xData.isEmpty()) {
        appendNewXPoint();
    } else {
        while (m_ySeries[seriesIndex].size() < m_xData.size())
            m_ySeries[seriesIndex].append(qQNaN());

        // 如果当前系列在“最新 X”位置已有值，则开启一个新的 X 点，
        // 否则把新值写到当前最新 X 位置，保证多源数据横坐标对齐。
        if (!m_ySeries[seriesIndex].isEmpty() && !qIsNaN(m_ySeries[seriesIndex].last()))
            appendNewXPoint();
    }

    if (m_ySeries[seriesIndex].size() < m_xData.size())
        m_ySeries[seriesIndex].resize(m_xData.size());

    m_ySeries[seriesIndex][m_xData.size() - 1] = value;

    while (m_xData.size() > m_maxPoints) {
        m_xData.remove(0);
        for (int i = 0; i < m_ySeries.size(); ++i) {
            if (!m_ySeries[i].isEmpty())
                m_ySeries[i].remove(0);
        }
    }

    for (int i = 0; i < m_curves.size(); ++i) {
        if (m_curves.value(i))
            m_curves[i]->setSamples(m_xData, m_ySeries.value(i));
    }

    if (!m_xData.isEmpty()) {
        const double xmin = m_xData.first();
        const double xmax = m_xData.last();
        const double span = qMax(1.0, xmax - xmin);
        const double step = qMax(1.0, qRound(span / 5.0));
        m_plot->setAxisScale(QwtPlot::xBottom, xmin, xmax, step);
    }

    m_plot->setAxisAutoScale(QwtPlot::yLeft, false);
    m_plot->setAxisScale(QwtPlot::yLeft, m_yMin, m_yMax);
    m_plot->replot();
}
