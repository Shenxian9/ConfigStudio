#ifndef PROJECTFILEMANAGER_H
#define PROJECTFILEMANAGER_H

#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QVariantMap>
#include <QRect>
#include <QVector>

#include "Variable.h"
#include "datasrc/serialdatasource.h"

struct ProjectBindingData {
    QString varId;
    QString itemId;
    QString property;
};

struct ProjectCanvasItemData {
    QString id;
    QString type;
    QRect geometry;
    int zValue = 0;
    double rotation = 0.0;
    bool visible = true;
    QVariantMap properties;
    QVector<ProjectBindingData> bindings;
};

class ProjectFileManager
{
public:
    static constexpr int kCurrentVersion = 1;

    static QJsonObject serializeVariable(const Variable &var);
    static bool deserializeVariable(const QJsonObject &obj, Variable *var);

    static QJsonObject serializeSerialConfig(const SerialPortConfig &cfg);
    static bool deserializeSerialConfig(const QJsonObject &obj, SerialPortConfig *cfg);

    static QJsonObject serializeCanvasItem(const ProjectCanvasItemData &item);
    static bool deserializeCanvasItem(const QJsonObject &obj, ProjectCanvasItemData *item);

    static QJsonObject serializeVariantMap(const QVariantMap &map);
    static QVariantMap deserializeVariantMap(const QJsonObject &obj);

    static QJsonValue variantToJsonValue(const QVariant &value);
    static QVariant jsonValueToVariant(const QJsonValue &value);
};

#endif // PROJECTFILEMANAGER_H
