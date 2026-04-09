#include <QtTest>

#include <QTreeView>
#include <QTableView>

#include "../datasrc/modbusrtudatasource.h"
#include "../runtime/databindingmanager.h"
#include "../ui/mainwindow.h"

class FakeWriteBackend : public IVariableWriteBackend
{
public:
    bool enabled = true;
    bool shouldSucceed = true;
    int callCount = 0;
    QString lastVarId;
    QVariant lastValue;
    bool writeEnabled() const override { return enabled; }
    bool writeVariable(const QString &varId, const QVariant &value, QString *errorText) override
    {
        ++callCount;
        lastVarId = varId;
        lastValue = value;
        if (!shouldSucceed) {
            if (errorText) *errorText = "fake fail";
            return false;
        }
        return true;
    }
};

class TestWriteCanvasItem : public CanvasItem
{
    Q_OBJECT
public:
    QString type() const override { return "test"; }
    QVariantMap properties() const override { return {}; }
    void setPropertyValue(const QString &, const QVariant &v) override { lastValue = v; }
    QVariant lastValue;
};

class Phase4ModbusWriteTest : public QObject
{
    Q_OBJECT
private slots:
    void writableValidation_rules();
    void encoding_bool_uint16_int16();
    void publishValue_inModbusMode_triggersWrite();
    void publishSameValue_doesNotRepeatWrite();
    void publishEquivalentNumeric_doesNotRepeatWrite();
    void writeSuccess_keepsValue();
    void writeFailed_rollsBack();
    void simulatorMode_doesNotWrite();
    void numeric_onlySingleRegisterTypes();
    void switch_writeFailure_restoresState();
    void phase123Constraints_stillHold();
};

void Phase4ModbusWriteTest::writableValidation_rules()
{
    ModbusRtuDataSource ds;
    quint16 encoded = 0;
    QString err;

    Variable u16; u16.area = RegisterArea::HoldingRegister; u16.type = "uint16"; u16.readOnly = false; u16.count = 1; u16.address = 1;
    Variable i16 = u16; i16.type = "int16";
    Variable b = u16; b.type = "bool"; b.bitOffset = 0;
    Variable f32 = u16; f32.type = "float32"; f32.count = 2;
    Variable input = u16; input.area = RegisterArea::InputRegister;
    Variable ro = u16; ro.readOnly = true;

    QVERIFY(ds.encodeSingleRegisterWriteForTest(u16, 1, &encoded, &err));
    QVERIFY(ds.encodeSingleRegisterWriteForTest(i16, -1, &encoded, &err));
    QVERIFY(ds.encodeSingleRegisterWriteForTest(b, true, &encoded, &err));
    QVERIFY(!ds.encodeSingleRegisterWriteForTest(f32, 1.2, &encoded, &err));
    QVERIFY(!ds.encodeSingleRegisterWriteForTest(input, 1, &encoded, &err));
    QVERIFY(!ds.encodeSingleRegisterWriteForTest(ro, 1, &encoded, &err));
}

void Phase4ModbusWriteTest::encoding_bool_uint16_int16()
{
    ModbusRtuDataSource ds;
    quint16 encoded = 0;
    QString err;

    Variable b; b.area = RegisterArea::HoldingRegister; b.type = "bool"; b.readOnly = false; b.count = 1;
    QVERIFY(ds.encodeSingleRegisterWriteForTest(b, false, &encoded, &err)); QCOMPARE(encoded, static_cast<quint16>(0));
    QVERIFY(ds.encodeSingleRegisterWriteForTest(b, true, &encoded, &err)); QCOMPARE(encoded, static_cast<quint16>(1));

    Variable u; u.area = RegisterArea::HoldingRegister; u.type = "uint16"; u.readOnly = false; u.count = 1;
    QVERIFY(ds.encodeSingleRegisterWriteForTest(u, 12345, &encoded, &err)); QCOMPARE(encoded, static_cast<quint16>(12345));

    Variable i; i.area = RegisterArea::HoldingRegister; i.type = "int16"; i.readOnly = false; i.count = 1;
    QVERIFY(ds.encodeSingleRegisterWriteForTest(i, -1, &encoded, &err)); QCOMPARE(encoded, static_cast<quint16>(0xFFFF));
    QVERIFY(ds.encodeSingleRegisterWriteForTest(i, -123, &encoded, &err)); QCOMPARE(encoded, static_cast<quint16>(static_cast<qint16>(-123)));
}

void Phase4ModbusWriteTest::publishValue_inModbusMode_triggersWrite()
{
    VariableModel model;
    Variable v; v.id = "W1"; v.value = 10; model.addVariable(v);
    DataBindingManager mgr(&model);
    FakeWriteBackend backend;
    backend.enabled = true;
    backend.shouldSucceed = true;
    mgr.setWriteBackend(&backend);

    QVERIFY(mgr.publishValue("W1", 20));
    QCOMPARE(backend.callCount, 1);
    QCOMPARE(backend.lastVarId, QString("W1"));
    QCOMPARE(backend.lastValue.toInt(), 20);
}

void Phase4ModbusWriteTest::publishSameValue_doesNotRepeatWrite()
{
    VariableModel model;
    Variable v; v.id = "W1R"; v.value = 10; model.addVariable(v);
    DataBindingManager mgr(&model);
    FakeWriteBackend backend;
    backend.enabled = true;
    backend.shouldSucceed = true;
    mgr.setWriteBackend(&backend);

    QVERIFY(mgr.publishValue("W1R", 20));
    QCOMPARE(backend.callCount, 1);
    QVERIFY(mgr.publishValue("W1R", 20));
    QCOMPARE(backend.callCount, 1);
}

void Phase4ModbusWriteTest::publishEquivalentNumeric_doesNotRepeatWrite()
{
    VariableModel model;
    Variable v; v.id = "W1N"; v.value = 10.0; model.addVariable(v);
    DataBindingManager mgr(&model);
    FakeWriteBackend backend;
    backend.enabled = true;
    backend.shouldSucceed = true;
    mgr.setWriteBackend(&backend);

    QVERIFY(mgr.publishValue("W1N", QString("20.0")));
    QCOMPARE(backend.callCount, 1);
    QVERIFY(mgr.publishValue("W1N", 20));
    QCOMPARE(backend.callCount, 1);
}

void Phase4ModbusWriteTest::writeSuccess_keepsValue()
{
    VariableModel model;
    Variable v; v.id = "W2"; v.value = 10; model.addVariable(v);
    DataBindingManager mgr(&model);
    FakeWriteBackend backend;
    backend.enabled = true;
    backend.shouldSucceed = true;
    mgr.setWriteBackend(&backend);

    QVERIFY(mgr.publishValue("W2", 20));
    QVariant out;
    QVERIFY(model.valueById("W2", &out));
    QCOMPARE(out.toInt(), 20);
}

void Phase4ModbusWriteTest::writeFailed_rollsBack()
{
    VariableModel model;
    Variable v; v.id = "W3"; v.value = 10; model.addVariable(v);
    DataBindingManager mgr(&model);
    FakeWriteBackend backend;
    backend.enabled = true;
    backend.shouldSucceed = false;
    mgr.setWriteBackend(&backend);

    TestWriteCanvasItem item;
    mgr.bind("W3", &item, "value");

    QVERIFY(!mgr.publishValue("W3", 20));
    QVariant out;
    QVERIFY(model.valueById("W3", &out));
    QCOMPARE(out.toInt(), 10);
    QCOMPARE(item.lastValue.toInt(), 10);
}

void Phase4ModbusWriteTest::simulatorMode_doesNotWrite()
{
    VariableModel model;
    Variable v; v.id = "W4"; v.value = 1; model.addVariable(v);
    DataBindingManager mgr(&model);
    FakeWriteBackend backend;
    backend.enabled = false;
    mgr.setWriteBackend(&backend);

    QVERIFY(mgr.publishValue("W4", 2));
    QCOMPARE(backend.callCount, 0);
}

void Phase4ModbusWriteTest::numeric_onlySingleRegisterTypes()
{
    ModbusRtuDataSource ds;
    quint16 encoded = 0;
    QString err;

    Variable ok; ok.area = RegisterArea::HoldingRegister; ok.type = "uint16"; ok.readOnly = false; ok.count = 1;
    Variable bad = ok; bad.type = "float32"; bad.count = 2;

    QVERIFY(ds.encodeSingleRegisterWriteForTest(ok, 88, &encoded, &err));
    QVERIFY(!ds.encodeSingleRegisterWriteForTest(bad, 12.3, &encoded, &err));
}

void Phase4ModbusWriteTest::switch_writeFailure_restoresState()
{
    VariableModel model;
    Variable v; v.id = "SW1"; v.type = "bool"; v.value = false; model.addVariable(v);
    DataBindingManager mgr(&model);
    FakeWriteBackend backend;
    backend.enabled = true;
    backend.shouldSucceed = false;
    mgr.setWriteBackend(&backend);

    TestWriteCanvasItem item;
    mgr.bind("SW1", &item, "on");

    QVERIFY(!mgr.publishValue("SW1", true));
    QVariant out;
    QVERIFY(model.valueById("SW1", &out));
    QCOMPARE(out.toBool(), false);
    QCOMPARE(item.lastValue.toBool(), false);
}

void Phase4ModbusWriteTest::phase123Constraints_stillHold()
{
    MainWindow w;
    w.show();
    QCoreApplication::processEvents();
    QVERIFY(!w.findChild<QPushButton *>("mappingAddButton"));
    auto *tree = w.findChild<QTreeView *>("treeView");
    QVERIFY(tree && tree->model());
    QStringList texts;
    std::function<void(const QModelIndex&)> walk = [&](const QModelIndex &p) {
        for (int r = 0; r < tree->model()->rowCount(p); ++r) {
            QModelIndex idx = tree->model()->index(r, 0, p);
            texts << idx.data().toString();
            walk(idx);
        }
    };
    walk({});
    QVERIFY(!texts.contains("Mappings"));
    auto *view = w.findChild<QTableView *>("variableView");
    QVERIFY(view);
    QCOMPARE(view->model(), static_cast<QAbstractItemModel *>(w.variableModelForTest()));
}

int runPhase4ModbusWriteTests(int argc, char *argv[])
{
    Phase4ModbusWriteTest tc;
    return QTest::qExec(&tc, argc, argv);
}

#include "tst_phase4_modbus_write.moc"
