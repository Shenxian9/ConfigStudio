#ifndef PROJECTFILEMANAGER_H
#define PROJECTFILEMANAGER_H

#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QVariantMap>
#include <QVector>

#include "model/Variable.h"
#include "datasrc/serialdatasource.h"

struct ProjectBindingData {
    QString varId;
    QString property;
};

struct ProjectItemData {
    QString id;
    QString type;
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
    double z = 0.0;
    double rotation = 0.0;
    bool visible = true;
    QVariantMap properties;
    QVector<ProjectBindingData> bindings;
};

struct ProjectData {
    int version = 1;
    int canvasWidth = 0;
    int canvasHeight = 0;
    QVector<ProjectItemData> items;
    QVector<Variable> variables;
    SerialPortConfig modbus;
    QJsonObject uiState;
};

class ProjectFileManager
{
public:
    static constexpr int kCurrentVersion = 1;

    static bool saveProject(const QString &filePath, const ProjectData &project, QString *errorText);
    static bool loadProject(const QString &filePath, ProjectData *project, QString *errorText);

private:
    static QJsonObject toJson(const ProjectData &project);
    static bool fromJson(const QJsonObject &obj, ProjectData *project, QString *errorText);

    static QJsonValue variantToJson(const QVariant &v);
    static QVariant jsonToVariant(const QJsonValue &value);

    static QJsonObject variableToJson(const Variable &var);
    static Variable variableFromJson(const QJsonObject &obj);

    static QJsonObject modbusToJson(const SerialPortConfig &cfg);
    static SerialPortConfig modbusFromJson(const QJsonObject &obj);
};

#endif // PROJECTFILEMANAGER_H
