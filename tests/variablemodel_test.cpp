#include <QtTest>
#include <QSignalSpy>

#include "../model/variablemodel.h"

class VariableModelTest : public QObject
{
    Q_OBJECT

private slots:
    void addVariable_exposesDataAndHeaders();
    void setData_updatesEditableColumns();
    void updateValueById_emitsSignalAndStoresValue();
    void valueById_handlesLookupAndNullOutput();
};

void VariableModelTest::addVariable_exposesDataAndHeaders()
{
    VariableModel model;
    Variable var;
    var.id = "V001";
    var.name = "Pressure";
    var.deviceId = "PLC-A";
    var.value = 12.5;
    var.unit = "bar";
    var.address = 100;
    var.lowLimit = 1.0;
    var.highLimit = 30.0;
    var.strategy = DataStrategy::Sine;

    model.addVariable(var);

    QCOMPARE(model.rowCount({}), 1);
    QCOMPARE(model.columnCount({}), VariableModel::ColumnCount);

    QCOMPARE(model.data(model.index(0, VariableModel::ColId), Qt::DisplayRole).toString(), QString("V001"));
    QCOMPARE(model.data(model.index(0, VariableModel::ColName), Qt::DisplayRole).toString(), QString("Pressure"));
    QCOMPARE(model.data(model.index(0, VariableModel::ColStrategy), Qt::DisplayRole).toInt(), static_cast<int>(DataStrategy::Sine));

    QCOMPARE(model.headerData(VariableModel::ColId, Qt::Horizontal, Qt::DisplayRole).toString(), QString("ID"));
    QCOMPARE(model.headerData(VariableModel::ColHighLimit, Qt::Horizontal, Qt::DisplayRole).toString(), QString("HighLimit"));
}

void VariableModelTest::setData_updatesEditableColumns()
{
    VariableModel model;
    Variable var;
    var.id = "V002";
    var.value = 20;
    model.addVariable(var);

    const QModelIndex idIndex = model.index(0, VariableModel::ColId);
    const QModelIndex valueIndex = model.index(0, VariableModel::ColValue);

    QVERIFY(model.flags(idIndex) & Qt::ItemIsEditable);
    QVERIFY(!(model.flags(valueIndex) & Qt::ItemIsEditable));

    QVERIFY(model.setData(idIndex, "V002-NEW", Qt::EditRole));
    QCOMPARE(model.data(idIndex, Qt::DisplayRole).toString(), QString("V002-NEW"));

    QVERIFY(model.setData(model.index(0, VariableModel::ColAddress), 123, Qt::EditRole));
    QCOMPARE(model.data(model.index(0, VariableModel::ColAddress), Qt::DisplayRole).toInt(), 123);

    QVERIFY(!model.setData(model.index(2, VariableModel::ColName), "X", Qt::EditRole));
    QVERIFY(!model.setData(model.index(0, VariableModel::ColName), "X", Qt::DisplayRole));
}

void VariableModelTest::updateValueById_emitsSignalAndStoresValue()
{
    VariableModel model;
    Variable var;
    var.id = "V003";
    model.addVariable(var);

    QSignalSpy spy(&model, &VariableModel::variableValueChanged);

    QVERIFY(model.updateValueById("V003", 88.8));
    QCOMPARE(spy.count(), 1);

    const QList<QVariant> args = spy.takeFirst();
    QCOMPARE(args.at(0).toString(), QString("V003"));
    QCOMPARE(args.at(1).toDouble(), 88.8);
    QCOMPARE(model.data(model.index(0, VariableModel::ColValue), Qt::DisplayRole).toDouble(), 88.8);

    QVERIFY(!model.updateValueById("NOT_FOUND", 1));
}

void VariableModelTest::valueById_handlesLookupAndNullOutput()
{
    VariableModel model;
    Variable var;
    var.id = "V004";
    var.value = "RUN";
    model.addVariable(var);

    QVariant out;
    QVERIFY(model.valueById("V004", &out));
    QCOMPARE(out.toString(), QString("RUN"));

    QVERIFY(!model.valueById("MISSING", &out));
    QVERIFY(!model.valueById("V004", nullptr));
}

QTEST_MAIN(VariableModelTest)
#include "variablemodel_test.moc"
