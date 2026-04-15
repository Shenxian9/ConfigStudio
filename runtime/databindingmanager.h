#ifndef DATABINDINGMANAGER_H
#define DATABINDINGMANAGER_H

#pragma once
#include <QObject>
#include <QHash>
#include <QPointer>
#include "model/variablemodel.h"
#include "canvas/canvasitem.h"
#include "variablewritebackend.h"

class DataBindingManager : public QObject
{
    Q_OBJECT
public:
    struct BindingInfo {
        QString varId;
        QString itemId;
        QString property;
    };

    explicit DataBindingManager(VariableModel* model, QObject* parent = nullptr);

    void bind(const QString& varId, CanvasItem* item, const QString& property);
    void unbind(const QString& varId, CanvasItem* item, const QString& property);
    void setWriteBackend(IVariableWriteBackend *backend) { m_writeBackend = backend; }
    bool publishValue(const QString& varId, const QVariant& value);
    bool currentValue(const QString& varId, QVariant* outValue) const;
    QList<BindingInfo> bindingInfos() const;
    void clearBindings();

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
