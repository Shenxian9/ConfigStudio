#ifndef DATABINDINGMANAGER_H
#define DATABINDINGMANAGER_H

#pragma once
#include <QObject>
#include <QHash>
#include <QPointer>
#include <QVector>
#include "model/variablemodel.h"
#include "canvas/canvasitem.h"
#include "variablewritebackend.h"

class DataBindingManager : public QObject
{
    Q_OBJECT
public:
    explicit DataBindingManager(VariableModel* model, QObject* parent = nullptr);
    struct BindingSnapshot {
        QString varId;
        QString itemId;
        QString property;
    };

    void bind(const QString& varId, CanvasItem* item, const QString& property);
    void unbind(const QString& varId, CanvasItem* item, const QString& property);
    void clear();
    QVector<BindingSnapshot> snapshot() const;
    void setWriteBackend(IVariableWriteBackend *backend) { m_writeBackend = backend; }
    bool publishValue(const QString& varId, const QVariant& value);
    bool currentValue(const QString& varId, QVariant* outValue) const;

private slots:
    void onVariableChanged(const QString& varId, const QVariant& value);

private:
    struct Binding {
        QPointer<CanvasItem> item;   // ⭐ 自动失效的安全指针
        QString property;
    };

    VariableModel* m_model;
    QHash<QString, QList<Binding>> m_bindings; // varId → 多个控件
    IVariableWriteBackend *m_writeBackend = nullptr;
};
#endif // DATABINDINGMANAGER_H
