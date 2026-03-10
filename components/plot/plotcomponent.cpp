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

    // 创建曲线对象
    m_curve = new QwtPlotCurve("Curve");
    m_curve->setRenderHint(QwtPlotItem::RenderAntialiased, true);
    m_curve->attach(m_plot);

    // 初始化空数据
    m_xData.reserve(m_maxPoints);
    m_yData.reserve(m_maxPoints);

}

QVariantMap PlotComponent::properties() const
{
    QVariantMap map;
    map["title"] = m_plot->title().text();
    map["xAxisTitle"] = m_plot->axisTitle(QwtPlot::xBottom).text();
    map["yAxisTitle"] = m_plot->axisTitle(QwtPlot::yLeft).text();
    map["xMin"] = m_plot->axisScaleDiv(QwtPlot::xBottom).lowerBound();
    map["xMax"] = m_plot->axisScaleDiv(QwtPlot::xBottom).upperBound();
    map["varId"] = m_varId;   // ⭐ 添加 varId 属性
    map["maxPoints"] = m_maxPoints;
    return map;
}

void PlotComponent::setPropertyValue(const QString& key, const QVariant& v)
{
    if (key == "value") {
        appendValue(v.toDouble());  // 流式更新
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
    else if (key == "varId") {
        QString newVarId = v.toString();
        if (newVarId == m_varId)
            return; // 相同 id 不重复绑定

        // ⭐ 如果之前绑定过，先解绑
        if (!m_varId.isEmpty() && m_bindingMgr) {
            m_bindingMgr->unbind(m_varId, this, "value");
        }

        m_varId = newVarId;

        if (m_bindingMgr && !m_varId.isEmpty()) {
            m_bindingMgr->bind(m_varId, this, "value");
            qDebug() << "PlotComponent bound to variable" << m_varId;
        }
    }
    else if (key == "maxPoints") {      // ⭐ 新增处理
        int mp = v.toInt();
        if (mp > 0)
            m_maxPoints = mp;
    }
}


void PlotComponent::resizeEvent(QResizeEvent* event)
{
    if (m_plot)
        m_plot->setGeometry(rect());
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

    const int count = qMin(m_maxPoints, m_xData.size());
    m_historyTable->setRowCount(count);

    const int start = m_xData.size() - count;
    for (int row = 0; row < count; ++row) {
        const int idx = start + row;
        m_historyTable->setItem(row, 0, new QTableWidgetItem(QString::number(row + 1)));
        m_historyTable->setItem(row, 1, new QTableWidgetItem(QString::number(m_xData[idx], 'f', 2)));
        m_historyTable->setItem(row, 2, new QTableWidgetItem(QString::number(m_yData[idx], 'f', 2)));
    }
}

void PlotComponent::appendValue(double value)
{
    double x = m_xData.isEmpty() ? 0 : m_xData.last() + 1;

    m_xData.append(x);
    m_yData.append(value);

    // ⭐ 保持不超过最大点数
    while (m_xData.size() > m_maxPoints) {
        m_xData.remove(0);
        m_yData.remove(0);
    }

    m_curve->setSamples(m_xData, m_yData);
    m_plot->replot();
    m_plot->setAxisScale(QwtPlot::xBottom, m_xData.first(), m_xData.last());
}
