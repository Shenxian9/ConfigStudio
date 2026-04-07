#ifndef MODBUSMAPPINGDEFS_H
#define MODBUSMAPPINGDEFS_H

#pragma once

#include <QString>
#include <QList>

#include "serialdatasource.h"

enum class ModbusPointKind {
    ReadOnly,
    ReadWrite,
    Command,
};

struct PollGroupDefinition {
    QString id;
    QString name;
    bool enabled = true;
    int intervalMs = 1000;
    int priority = 0;
    QString description;
};

struct ModbusPointDefinition {
    QString id;
    QString name;
    QString varId;
    bool enabled = true;

    ModbusPointKind kind = ModbusPointKind::ReadOnly;

    int slaveId = 1;
    int functionCode = 3;
    int address = 0;
    int quantity = 1;

    bool readable = true;
    bool writable = false;

    QString dataType = QStringLiteral("u16");
    QString byteOrder = QStringLiteral("BigEndian");
    QString wordOrder = QStringLiteral("Normal");
    double scale = 1.0;
    double offset = 0.0;

    QString pollGroupId;
    QString writeStrategy = QStringLiteral("immediate");
    QString description;
};

struct ModbusMappingConfig {
    SerialPortConfig connection;
    QList<PollGroupDefinition> pollGroups;
    QList<ModbusPointDefinition> points;
};

QString toDisplayString(ModbusPointKind kind);
void applyKindDefaults(ModbusPointDefinition &def);

#endif // MODBUSMAPPINGDEFS_H
