#ifndef PROJECTFILEMANAGER_H
#define PROJECTFILEMANAGER_H

#pragma once

#include <QString>
#include <QStringList>
#include <QVector>
#include <QJsonObject>
#include <QJsonArray>

#include "canvas/canvasview.h"
#include "datasrc/serialdatasource.h"
#include "runtime/databindingmanager.h"
#include "variablemodel.h"

class ProjectFileManager
{
public:
    static constexpr int kProjectVersion = 1;

    bool saveProject(const QString &filePath,
                     CanvasView *canvas,
                     VariableModel *variableModel,
                     DataBindingManager *bindingMgr,
                     const QVector<SerialPortConfig> &modbusConfigs,
                     const SerialPortConfig &activeConfig,
                     QString *errorText) const;

    bool loadProject(const QString &filePath,
                     CanvasView *canvas,
                     VariableModel *variableModel,
                     DataBindingManager *bindingMgr,
                     QVector<SerialPortConfig> *modbusConfigs,
                     SerialPortConfig *activeConfig,
                     QStringList *warnings,
                     QString *errorText) const;

private:
    QJsonObject serializeVariable(const Variable &var) const;
    bool deserializeVariable(const QJsonObject &obj, Variable *var) const;

    QJsonObject serializeModbusConfig(const SerialPortConfig &cfg) const;
    SerialPortConfig deserializeModbusConfig(const QJsonObject &obj) const;

    QJsonObject serializeItem(CanvasItem *item, int zIndex) const;
    void applyItemProperties(CanvasItem *item, const QJsonObject &itemObj, QStringList *warnings) const;
    void applyBindings(CanvasItem *item, const QJsonArray &bindings, DataBindingManager *bindingMgr, QStringList *warnings) const;

    QString ensureItemId(CanvasItem *item, int fallbackIndex) const;
    static QString bindingPropertyToVarKey(const QString &property);
};

#endif // PROJECTFILEMANAGER_H
