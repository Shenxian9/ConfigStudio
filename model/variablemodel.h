#ifndef VARIABLEMODEL_H
#define VARIABLEMODEL_H

// model/VariableModel.h
#pragma once
#include <QAbstractTableModel>
#include <QDateTime>
#include <QDebug>
#include "Variable.h"

class VariableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit VariableModel(QObject* parent = nullptr);

    enum Columns {
        ColId,
        ColName,
        ColDevice,
        ColType,
        ColArea,
        ColAddress,
        ColCount,
        ColValue,
        ColUnit,
        ColBitOffset,
        ColScale,
        ColReadOnly,
        ColEndianness,
        ColLowLimit,
        ColHighLimit,
        ColStrategy,
        ColumnCount
    };



    int rowCount(const QModelIndex&) const override;
    int columnCount(const QModelIndex&) const override { return ColumnCount; }
    QVariant data(const QModelIndex&, int role) const override;
    QVariant headerData(int, Qt::Orientation, int role) const override;
    bool setData(const QModelIndex&, const QVariant&, int role) override;
    Qt::ItemFlags flags(const QModelIndex&) const override;

    void addVariable(const Variable& var);
    Variable& variableAt(int row);
    const Variable& variableAt(int row) const;
    bool setVariableAt(int row, const Variable &var);
    bool removeVariableAt(int row);
    bool hasVariableId(const QString &id, int excludeRow = -1) const;
    int rowById(const QString &id) const;
    void updateValue(int row, const QVariant& value);
    bool updateValueById(const QString& varId, const QVariant& value);
    bool valueById(const QString& varId, QVariant* outValue) const;
    QStringList variableIds() const;

signals:
    void variableValueChanged(const QString& varId, const QVariant& value);

private:
    QVector<Variable> m_vars;
};

#endif // VARIABLEMODEL_H
