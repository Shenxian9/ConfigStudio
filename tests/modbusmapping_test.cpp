#include <QtTest>

#include "../datasrc/modbusmappingdefs.h"

class ModbusMappingTest : public QObject
{
    Q_OBJECT

private slots:
    void pollGroup_fieldsReadableWritable();
    void point_fieldsReadableWritable();
    void pointKind_isExplicitlyDistinguished();
    void mappingConfig_crudByIdWorks();
    void pointPollGroupAssignment_isPreserved();
};

void ModbusMappingTest::pollGroup_fieldsReadableWritable()
{
    PollGroupDefinition group;
    group.id = "fast_1";
    group.name = "Fast";
    group.enabled = false;
    group.intervalMs = 200;
    group.priority = 2;
    group.description = "for high-frequency values";

    QCOMPARE(group.id, QString("fast_1"));
    QCOMPARE(group.name, QString("Fast"));
    QCOMPARE(group.enabled, false);
    QCOMPARE(group.intervalMs, 200);
    QCOMPARE(group.priority, 2);
    QCOMPARE(group.description, QString("for high-frequency values"));
}

void ModbusMappingTest::point_fieldsReadableWritable()
{
    ModbusPointDefinition point;
    point.id = "p_flow";
    point.name = "Flow";
    point.varId = "var_flow";
    point.enabled = false;
    point.slaveId = 2;
    point.functionCode = 4;
    point.address = 120;
    point.quantity = 2;
    point.readable = true;
    point.writable = false;
    point.dataType = "float32";
    point.byteOrder = "LittleEndian";
    point.wordOrder = "Swap";
    point.scale = 0.1;
    point.offset = 10.0;
    point.pollGroupId = "fast_1";
    point.writeStrategy = "debounce";
    point.description = "flow sensor";

    QCOMPARE(point.id, QString("p_flow"));
    QCOMPARE(point.name, QString("Flow"));
    QCOMPARE(point.varId, QString("var_flow"));
    QCOMPARE(point.enabled, false);
    QCOMPARE(point.slaveId, 2);
    QCOMPARE(point.functionCode, 4);
    QCOMPARE(point.address, 120);
    QCOMPARE(point.quantity, 2);
    QCOMPARE(point.readable, true);
    QCOMPARE(point.writable, false);
    QCOMPARE(point.dataType, QString("float32"));
    QCOMPARE(point.byteOrder, QString("LittleEndian"));
    QCOMPARE(point.wordOrder, QString("Swap"));
    QCOMPARE(point.scale, 0.1);
    QCOMPARE(point.offset, 10.0);
    QCOMPARE(point.pollGroupId, QString("fast_1"));
    QCOMPARE(point.writeStrategy, QString("debounce"));
    QCOMPARE(point.description, QString("flow sensor"));
}

void ModbusMappingTest::pointKind_isExplicitlyDistinguished()
{
    ModbusPointDefinition readOnly;
    readOnly.kind = ModbusPointKind::ReadOnly;
    applyKindDefaults(readOnly);

    ModbusPointDefinition readWrite;
    readWrite.kind = ModbusPointKind::ReadWrite;
    applyKindDefaults(readWrite);

    ModbusPointDefinition command;
    command.kind = ModbusPointKind::Command;
    applyKindDefaults(command);

    QCOMPARE(static_cast<int>(readOnly.kind), static_cast<int>(ModbusPointKind::ReadOnly));
    QCOMPARE(static_cast<int>(readWrite.kind), static_cast<int>(ModbusPointKind::ReadWrite));
    QCOMPARE(static_cast<int>(command.kind), static_cast<int>(ModbusPointKind::Command));

    QCOMPARE(readOnly.readable, true);
    QCOMPARE(readOnly.writable, false);
    QCOMPARE(readWrite.readable, true);
    QCOMPARE(readWrite.writable, true);
    QCOMPARE(command.readable, false);
    QCOMPARE(command.writable, true);
}

void ModbusMappingTest::mappingConfig_crudByIdWorks()
{
    ModbusMappingConfig config;

    PollGroupDefinition group;
    group.id = "fast_1";
    group.name = "Fast";
    QVERIFY(upsertPollGroup(config, group));
    QCOMPARE(config.pollGroups.size(), 1);
    QCOMPARE(findPollGroupIndexById(config, "fast_1"), 0);

    group.intervalMs = 250;
    QVERIFY(upsertPollGroup(config, group));
    QCOMPARE(config.pollGroups.size(), 1);
    QCOMPARE(config.pollGroups.at(0).intervalMs, 250);

    ModbusPointDefinition point;
    point.id = "p_flow";
    point.varId = "var_flow";
    point.kind = ModbusPointKind::ReadOnly;
    point.pollGroupId = "fast_1";
    QVERIFY(upsertPoint(config, point));
    QCOMPARE(config.points.size(), 1);
    QCOMPARE(findPointIndexById(config, "p_flow"), 0);

    point.address = 101;
    QVERIFY(upsertPoint(config, point));
    QCOMPARE(config.points.size(), 1);
    QCOMPARE(config.points.at(0).address, 101);

    QVERIFY(removePointById(config, "p_flow"));
    QCOMPARE(config.points.size(), 0);
    QVERIFY(removePollGroupById(config, "fast_1"));
    QCOMPARE(config.pollGroups.size(), 0);
}

void ModbusMappingTest::pointPollGroupAssignment_isPreserved()
{
    ModbusMappingConfig config;

    PollGroupDefinition group;
    group.id = "fast_1";
    QVERIFY(upsertPollGroup(config, group));

    ModbusPointDefinition point;
    point.id = "p_zero_cut";
    point.pollGroupId = "fast_1";
    QVERIFY(upsertPoint(config, point));

    const int idx = findPointIndexById(config, "p_zero_cut");
    QVERIFY(idx >= 0);
    QCOMPARE(config.points.at(idx).pollGroupId, QString("fast_1"));
}

QTEST_MAIN(ModbusMappingTest)
#include "modbusmapping_test.moc"
