#include "plotcomponent.h"
#include <qwt_text.h>
#include <qwt_scale_div.h>

#include <qwt_plot_curve.h>

#include <QDialog>
#include <QHeaderView>
#include <QTableWidget>
#include <QVBoxLayout>

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
    QDialog dlg(this);
    dlg.setWindowTitle(QString("Plot History (N=%1)").arg(m_maxPoints));
    dlg.setWindowFlags(dlg.windowFlags() | Qt::Popup);
    dlg.resize(420, 320);

    QVBoxLayout *layout = new QVBoxLayout(&dlg);
    QTableWidget *table = new QTableWidget(&dlg);
    table->setColumnCount(3);
    table->setHorizontalHeaderLabels({"#", "X", "Y"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->setVisible(false);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);

    const int count = qMin(m_maxPoints, m_xData.size());
    table->setRowCount(count);

    const int start = m_xData.size() - count;
    for (int row = 0; row < count; ++row) {
        const int idx = start + row;
        table->setItem(row, 0, new QTableWidgetItem(QString::number(row + 1)));
        table->setItem(row, 1, new QTableWidgetItem(QString::number(m_xData[idx], 'f', 2)));
        table->setItem(row, 2, new QTableWidgetItem(QString::number(m_yData[idx], 'f', 2)));
    }

    layout->addWidget(table);

    dlg.exec();
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
