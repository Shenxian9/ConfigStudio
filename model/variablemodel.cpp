// model/VariableModel.cpp
#include "variablemodel.h"
#include <QBrush>

VariableModel::VariableModel(QObject* parent)
    : QAbstractTableModel(parent)
{
}

// 行数
int VariableModel::rowCount(const QModelIndex&) const
{
    return m_vars.size();
}

// 数据展示
QVariant VariableModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= m_vars.size())
        return QVariant();

    const Variable& var = m_vars.at(index.row());

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (index.column()) {
        case ColId:        return var.id;
        case ColName:      return var.name;
        case ColDevice:    return var.deviceId;
        case ColValue:     return var.value;
        case ColUnit:      return var.unit;
        case ColAddress:   return var.address;
        case ColLowLimit:  return var.lowLimit;
        case ColHighLimit: return var.highLimit;
        case ColStrategy:  return static_cast<int>(var.strategy);
        }
    }
    return QVariant();
}



// 设置数据
bool VariableModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || index.row() >= m_vars.size())
        return false;

    Variable& var = m_vars[index.row()];

    if (role == Qt::EditRole) {
        switch (index.column()) {
        case ColId:        var.id = value.toString(); break;
        case ColName:      var.name = value.toString(); break;
        case ColDevice:    var.deviceId = value.toString(); break;
        case ColValue:     var.value = value; break;
        case ColUnit:      var.unit = value.toString(); break;
        case ColAddress:   var.address = value.toInt(); break;
        case ColLowLimit:  var.lowLimit = value.toDouble(); break;
        case ColHighLimit: var.highLimit = value.toDouble(); break;
        case ColStrategy:  var.strategy = static_cast<DataStrategy>(value.toInt()); break;
        default: return false;
        }
        emit dataChanged(index, index);
        return true;
    }
    return false;
}



// 列名
QVariant VariableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
        return QVariant();

    switch (section) {
    case ColId:        return "ID";
    case ColName:      return "Name";
    case ColDevice:    return "Device";
    case ColValue:     return "Value";
    case ColUnit:      return "Unit";
    case ColAddress:   return "Address";
    case ColLowLimit:  return "LowLimit";
    case ColHighLimit: return "HighLimit";
    case ColStrategy:  return "Strategy";
    }
    return QVariant();
}


// 可编辑列
Qt::ItemFlags VariableModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    Qt::ItemFlags f = Qt::ItemIsSelectable | Qt::ItemIsEnabled;

    if (index.column() != ColValue) // Value 由仿真驱动
        f |= Qt::ItemIsEditable;

    return f;
}



// 添加变量
void VariableModel::addVariable(const Variable& var)
{
    beginInsertRows(QModelIndex(), m_vars.size(), m_vars.size());
    m_vars.append(var);
    endInsertRows();
}

// 获取某行变量
Variable& VariableModel::variableAt(int row)
{
    Q_ASSERT(row >= 0 && row < m_vars.size());
    return m_vars[row];
}

void VariableModel::updateValue(int row, const QVariant& value)
{
    if (row < 0 || row >= m_vars.size())
        return;

    m_vars[row].value = value;
    m_vars[row].timestamp = QDateTime::currentMSecsSinceEpoch();

    QModelIndex idx = index(row, ColValue);
    emit dataChanged(idx, idx, {Qt::DisplayRole});
    //qDebug() << "Model emit:" << m_vars[row].id << value;
    emit variableValueChanged(m_vars[row].id, value);  // ⭐ 广播
}
