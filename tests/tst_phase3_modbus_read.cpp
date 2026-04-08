#include <QtTest>

#include <QComboBox>
#include <QPushButton>
#include <QTableView>
#include <QTreeView>
#include <functional>
#include <QtMath>

#include "../datasrc/modbusrtudatasource.h"
#include "../runtime/databindingmanager.h"
#include "../ui/mainwindow.h"

class TestCanvasItem : public CanvasItem
{
    Q_OBJECT
public:
    explicit TestCanvasItem(QWidget *parent = nullptr) : CanvasItem(parent) {}
    QString type() const override { return "test"; }
    QVariantMap properties() const override { return {}; }
    void setPropertyValue(const QString &key, const QVariant &v) override
    {
        lastKey = key;
        lastValue = v;
    }

    QString lastKey;
    QVariant lastValue;
};

class Phase3ModbusReadTest : public QObject
{
    Q_OBJECT
private slots:
    void decode_uint16_int16();
    void decode_float32_with_endianness();
    void decode_bool_with_bitOffset();
    void pollingFilter_keepsSupportedVariablesOnly();
    void processReadResult_updatesVariableModel();
    void dataBindingManager_distributesModelUpdate();
    void modeSwitch_preventsDoubleDriving();
    void phase12Constraints_noRegression();
};

void Phase3ModbusReadTest::decode_uint16_int16()
{
    Variable u; u.type = "uint16"; u.scale = 1.0;
    Variable s; s.type = "int16"; s.scale = 1.0;
    bool ok = false;
    QCOMPARE(ModbusRtuDataSource::decodeRegisters(u, {0xFFFE}, &ok).toDouble(), 65534.0);
    QVERIFY(ok);
    QCOMPARE(ModbusRtuDataSource::decodeRegisters(s, {0xFFFE}, &ok).toDouble(), -2.0);
    QVERIFY(ok);
}

void Phase3ModbusReadTest::decode_float32_with_endianness()
{
    Variable f1; f1.type = "float32"; f1.endianness = Endianness::BigEndian;
    Variable f2; f2.type = "float32"; f2.endianness = Endianness::BigEndianWordSwap;
    bool ok = false;
    const double a = ModbusRtuDataSource::decodeRegisters(f1, {0x4120, 0x0000}, &ok).toDouble();
    QVERIFY(ok);
    QVERIFY(qAbs(a - 10.0) < 0.001);
    const double b = ModbusRtuDataSource::decodeRegisters(f2, {0x0000, 0x4120}, &ok).toDouble();
    QVERIFY(ok);
    QVERIFY(qAbs(b - 10.0) < 0.001);
}

void Phase3ModbusReadTest::decode_bool_with_bitOffset()
{
    Variable v; v.type = "bool"; v.bitOffset = 3;
    bool ok = false;
    QCOMPARE(ModbusRtuDataSource::decodeRegisters(v, {0x0008}, &ok).toBool(), true);
    QVERIFY(ok);
    v.bitOffset = 2;
    QCOMPARE(ModbusRtuDataSource::decodeRegisters(v, {0x0008}, &ok).toBool(), false);
    QVERIFY(ok);
}

void Phase3ModbusReadTest::pollingFilter_keepsSupportedVariablesOnly()
{
    VariableModel model;
    Variable a; a.id = "a"; a.deviceId = "dev1"; a.area = RegisterArea::HoldingRegister; a.type = "uint16"; a.count = 1; a.address = 0;
    Variable b; b.id = "b"; b.deviceId = "dev1"; b.area = RegisterArea::InputRegister; b.type = "float32"; b.count = 2; b.address = 10;
    Variable c; c.id = "c"; c.deviceId = "dev1"; c.area = RegisterArea::Coil; c.type = "bool"; c.address = 1;
    Variable d; d.id = "d"; d.deviceId = "dev1"; d.area = RegisterArea::HoldingRegister; d.type = "uint16"; d.address = -1;
    Variable e; e.id = "e"; e.deviceId = "dev2"; e.area = RegisterArea::HoldingRegister; e.type = "uint16"; e.address = 2;
    model.addVariable(a); model.addVariable(b); model.addVariable(c); model.addVariable(d); model.addVariable(e);

    ModbusRtuDataSource ds;
    SerialPortConfig cfg; cfg.deviceId = "dev1";
    ds.setConfig(cfg);
    ds.setVariableModel(&model);
    const QVector<int> rows = ds.eligibleVariableRowsForTest();
    QCOMPARE(rows.size(), 2);
}

void Phase3ModbusReadTest::processReadResult_updatesVariableModel()
{
    VariableModel model;
    Variable v; v.id = "f32"; v.deviceId = "default"; v.type = "float32"; v.area = RegisterArea::HoldingRegister; v.count = 2;
    model.addVariable(v);

    ModbusRtuDataSource ds;
    ds.setVariableModel(&model);
    QString err;
    QVERIFY(ds.processReadResultForTest("f32", {0x4120, 0x0000}, &err));
    QVariant out;
    QVERIFY(model.valueById("f32", &out));
    QVERIFY(qAbs(out.toDouble() - 10.0) < 0.001);
}

void Phase3ModbusReadTest::dataBindingManager_distributesModelUpdate()
{
    VariableModel model;
    Variable v; v.id = "v1"; v.deviceId = "default"; model.addVariable(v);
    DataBindingManager mgr(&model);
    TestCanvasItem item;
    mgr.bind("v1", &item, "value");

    ModbusRtuDataSource ds;
    ds.setVariableModel(&model);
    QVERIFY(ds.processReadResultForTest("v1", {0x0011}, nullptr));
    QCOMPARE(item.lastKey, QString("value"));
    QCOMPARE(item.lastValue.toDouble(), 17.0);
}

void Phase3ModbusReadTest::modeSwitch_preventsDoubleDriving()
{
    MainWindow w;
    w.show();
    QCoreApplication::processEvents();

    auto *mode = w.findChild<QComboBox *>("dataSourceModeCombo");
    QVERIFY(mode);
    mode->setCurrentText("Modbus RTU");
    QCoreApplication::processEvents();
    QVERIFY(w.runtimeSimulatorForTest());
    QVERIFY(!w.runtimeSimulatorForTest()->isRunning());

    mode->setCurrentText("Simulator");
    QCoreApplication::processEvents();
    QVERIFY(w.runtimeSimulatorForTest()->isRunning());
}

void Phase3ModbusReadTest::phase12Constraints_noRegression()
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

int runPhase3ModbusReadTests(int argc, char *argv[])
{
    Phase3ModbusReadTest tc;
    return QTest::qExec(&tc, argc, argv);
}

#include "tst_phase3_modbus_read.moc"
