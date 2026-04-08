#include "modbusmappingdefs.h"

QString toDisplayString(ModbusPointKind kind)
{
    switch (kind) {
    case ModbusPointKind::ReadOnly:
        return QStringLiteral("ReadOnly");
    case ModbusPointKind::ReadWrite:
        return QStringLiteral("ReadWrite");
    case ModbusPointKind::Command:
        return QStringLiteral("Command");
    }
    return QStringLiteral("ReadOnly");
}

void applyKindDefaults(ModbusPointDefinition &def)
{
    switch (def.kind) {
    case ModbusPointKind::ReadOnly:
        def.readable = true;
        def.writable = false;
        if (def.writeStrategy.isEmpty())
            def.writeStrategy = QStringLiteral("immediate");
        break;
    case ModbusPointKind::ReadWrite:
        def.readable = true;
        def.writable = true;
        if (def.writeStrategy.isEmpty())
            def.writeStrategy = QStringLiteral("immediate");
        break;
    case ModbusPointKind::Command:
        def.readable = false;
        def.writable = true;
        if (def.writeStrategy.isEmpty())
            def.writeStrategy = QStringLiteral("submit");
        break;
    }
}

int findPollGroupIndexById(const ModbusMappingConfig &config, const QString &id)
{
    const QString key = id.trimmed();
    for (int i = 0; i < config.pollGroups.size(); ++i) {
        if (config.pollGroups.at(i).id == key)
            return i;
    }
    return -1;
}

int findPointIndexById(const ModbusMappingConfig &config, const QString &id)
{
    const QString key = id.trimmed();
    for (int i = 0; i < config.points.size(); ++i) {
        if (config.points.at(i).id == key)
            return i;
    }
    return -1;
}

bool upsertPollGroup(ModbusMappingConfig &config, const PollGroupDefinition &group)
{
    if (group.id.trimmed().isEmpty())
        return false;
    const int existing = findPollGroupIndexById(config, group.id);
    if (existing >= 0)
        config.pollGroups[existing] = group;
    else
        config.pollGroups.append(group);
    return true;
}

bool upsertPoint(ModbusMappingConfig &config, const ModbusPointDefinition &point)
{
    if (point.id.trimmed().isEmpty())
        return false;
    const int existing = findPointIndexById(config, point.id);
    if (existing >= 0)
        config.points[existing] = point;
    else
        config.points.append(point);
    return true;
}

bool removePollGroupById(ModbusMappingConfig &config, const QString &id)
{
    const int idx = findPollGroupIndexById(config, id);
    if (idx < 0)
        return false;
    config.pollGroups.removeAt(idx);
    return true;
}

bool removePointById(ModbusMappingConfig &config, const QString &id)
{
    const int idx = findPointIndexById(config, id);
    if (idx < 0)
        return false;
    config.points.removeAt(idx);
    return true;
}
