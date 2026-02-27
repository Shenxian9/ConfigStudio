#ifndef DATABINDINGMANAGER_H
#define DATABINDINGMANAGER_H

#pragma once
#include <QObject>
#include <QHash>
#include <QPointer>
#include "model/variablemodel.h"
#include "canvas/canvasitem.h"

class DataBindingManager : public QObject
{
    Q_OBJECT
public:
    explicit DataBindingManager(VariableModel* model, QObject* parent = nullptr);

    void bind(const QString& varId, CanvasItem* item, const QString& property);
    void unbind(const QString& varId, CanvasItem* item, const QString& property);

private slots:
    void onVariableChanged(const QString& varId, const QVariant& value);

private:
    struct Binding {
        QPointer<CanvasItem> item;   // ⭐ 自动失效的安全指针
        QString property;
    };

    VariableModel* m_model;
    QHash<QString, QList<Binding>> m_bindings; // varId → 多个控件
};
#endif // DATABINDINGMANAGER_H
