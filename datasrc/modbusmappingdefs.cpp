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
