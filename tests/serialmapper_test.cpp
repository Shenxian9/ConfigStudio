#include <QtTest>
#include <QSignalSpy>

#include "../datasrc/serialdatasource.h"
#include "../model/variablemodel.h"

class SerialMapperTest : public QObject
{
    Q_OBJECT

private slots:
    void parseFrame_updatesMappedVariables();
    void parseFrame_supportsCommaDelimiterAndStringValue();
    void parseFrame_ignoresUnknownOrInvalidPairs();
};

void SerialMapperTest::parseFrame_updatesMappedVariables()
{
    VariableModel model;
    Variable t; t.id = "temp"; model.addVariable(t);
    Variable p; p.id = "press"; model.addVariable(p);

    SerialVariableMapper mapper(&model);
    mapper.setBinding("T", "temp");
    mapper.setBinding("P", "press");

    QSignalSpy spy(&model, &VariableModel::variableValueChanged);

    mapper.onFrameReceived("T=12.5;P=101.3");

    QCOMPARE(spy.count(), 2);

    QVariant out;
    QVERIFY(model.valueById("temp", &out));
    QCOMPARE(out.toDouble(), 12.5);
    QVERIFY(model.valueById("press", &out));
    QCOMPARE(out.toDouble(), 101.3);
}

void SerialMapperTest::parseFrame_supportsCommaDelimiterAndStringValue()
{
    VariableModel model;
    Variable s; s.id = "status"; model.addVariable(s);

    SerialVariableMapper mapper(&model);
    mapper.setBinding("STATE", "status");

    mapper.onFrameReceived("STATE=RUN,OTHER=1");

    QVariant out;
    QVERIFY(model.valueById("status", &out));
    QCOMPARE(out.toString(), QString("RUN"));
}

void SerialMapperTest::parseFrame_ignoresUnknownOrInvalidPairs()
{
    VariableModel model;
    Variable v; v.id = "v1"; v.value = 7; model.addVariable(v);

    SerialVariableMapper mapper(&model);
    mapper.setBinding("KEY", "v1");

    mapper.onFrameReceived("BADSEGMENT;MISSING=;UNKNOWN=11");

    QVariant out;
    QVERIFY(model.valueById("v1", &out));
    QCOMPARE(out.toInt(), 7);
}

QTEST_MAIN(SerialMapperTest)
#include "serialmapper_test.moc"
