// model/VariableModel.cpp
#include "variablemodel.h"
#include <QBrush>

namespace {
QString registerAreaToString(RegisterArea area)
{
    switch (area) {
    case RegisterArea::Coil: return "Coil";
    case RegisterArea::DiscreteInput: return "DiscreteInput";
    case RegisterArea::InputRegister: return "InputRegister";
    case RegisterArea::HoldingRegister:
    default:
        return "HoldingRegister";
    }
}

RegisterArea registerAreaFromString(const QString &area)
{
    const QString normalized = area.trimmed();
    if (normalized.compare("Coil", Qt::CaseInsensitive) == 0) return RegisterArea::Coil;
    if (normalized.compare("DiscreteInput", Qt::CaseInsensitive) == 0) return RegisterArea::DiscreteInput;
    if (normalized.compare("InputRegister", Qt::CaseInsensitive) == 0) return RegisterArea::InputRegister;
    return RegisterArea::HoldingRegister;
}

QString endiannessToString(Endianness e)
{
    switch (e) {
    case Endianness::BigEndianWordSwap: return "BigEndianWordSwap";
    case Endianness::LittleEndian: return "LittleEndian";
    case Endianness::LittleEndianByteSwap: return "LittleEndianByteSwap";
    case Endianness::BigEndian:
    default:
        return "BigEndian";
    }
}

Endianness endiannessFromString(const QString &v)
{
    if (v.trimmed().compare("BigEndianWordSwap", Qt::CaseInsensitive) == 0)
        return Endianness::BigEndianWordSwap;
    if (v.trimmed().compare("LittleEndian", Qt::CaseInsensitive) == 0)
        return Endianness::LittleEndian;
    if (v.trimmed().compare("LittleEndianByteSwap", Qt::CaseInsensitive) == 0)
        return Endianness::LittleEndianByteSwap;
    return Endianness::BigEndian;
}
}

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
        case ColType:      return var.type;
        case ColArea:      return registerAreaToString(var.area);
        case ColAddress:   return var.address;
        case ColCount:     return var.count;
        case ColValue:     return var.value;
        case ColUnit:      return var.unit;
        case ColBitOffset: return var.bitOffset;
        case ColScale:     return var.scale;
        case ColReadOnly:  return var.readOnly;
        case ColEndianness:return endiannessToString(var.endianness);
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
        case ColType:      var.type = value.toString(); break;
        case ColArea:      var.area = registerAreaFromString(value.toString()); break;
        case ColAddress:   var.address = value.toInt(); break;
        case ColCount:     var.count = qMax(1, value.toInt()); break;
        case ColValue:     var.value = value; break;
        case ColUnit:      var.unit = value.toString(); break;
        case ColBitOffset: var.bitOffset = qMax(0, value.toInt()); break;
        case ColScale:     var.scale = qMax(0.000001, value.toDouble()); break;
        case ColReadOnly:  var.readOnly = value.toBool(); break;
        case ColEndianness:var.endianness = endiannessFromString(value.toString()); break;
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
    case ColType:      return "Type";
    case ColArea:      return "Area";
    case ColAddress:   return "Address";
    case ColCount:     return "Count";
    case ColValue:     return "Value";
    case ColUnit:      return "Unit";
    case ColBitOffset: return "BitOffset";
    case ColScale:     return "Scale";
    case ColReadOnly:  return "ReadOnly";
    case ColEndianness:return "Endianness";
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

const Variable& VariableModel::variableAt(int row) const
{
    Q_ASSERT(row >= 0 && row < m_vars.size());
    return m_vars[row];
}

bool VariableModel::setVariableAt(int row, const Variable &var)
{
    if (row < 0 || row >= m_vars.size())
        return false;
    m_vars[row] = var;
    emit dataChanged(index(row, 0), index(row, ColumnCount - 1));
    return true;
}

bool VariableModel::removeVariableAt(int row)
{
    if (row < 0 || row >= m_vars.size())
        return false;
    beginRemoveRows(QModelIndex(), row, row);
    m_vars.removeAt(row);
    endRemoveRows();
    return true;
}

bool VariableModel::hasVariableId(const QString &id, int excludeRow) const
{
    const QString target = id.trimmed();
    if (target.isEmpty())
        return false;
    for (int i = 0; i < m_vars.size(); ++i) {
        if (i == excludeRow)
            continue;
        if (m_vars.at(i).id == target)
            return true;
    }
    return false;
}

int VariableModel::rowById(const QString &id) const
{
    const QString target = id.trimmed();
    for (int i = 0; i < m_vars.size(); ++i) {
        if (m_vars.at(i).id == target)
            return i;
    }
    return -1;
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


bool VariableModel::updateValueById(const QString& varId, const QVariant& value)
{
    for (int row = 0; row < m_vars.size(); ++row) {
        if (m_vars[row].id == varId) {
            updateValue(row, value);
            return true;
        }
    }
    return false;
}


bool VariableModel::valueById(const QString& varId, QVariant* outValue) const
{
    if (!outValue)
        return false;

    for (int row = 0; row < m_vars.size(); ++row) {
        if (m_vars[row].id == varId) {
            *outValue = m_vars[row].value;
            return true;
        }
    }
    return false;
}

QStringList VariableModel::variableIds() const
{
    QStringList ids;
    ids.reserve(m_vars.size());
    for (const Variable &var : m_vars)
        ids.append(var.id);
    return ids;
}
