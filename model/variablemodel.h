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
        ColId,          // ⭐ 新增第一列
        ColName,
        ColDevice,
        ColValue,
        ColUnit,
        ColAddress,     // 保留一个通讯核心字段
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
    void updateValue(int row, const QVariant& value);

signals:
    void variableValueChanged(const QString& varId, const QVariant& value);

private:
    QVector<Variable> m_vars;
};

#endif // VARIABLEMODEL_H
