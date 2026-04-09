#include <QtTest>

#include <QAbstractItemModel>
#include <QApplication>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTableView>
#include <QTreeView>
#include <QTimer>
#include <QMessageBox>

#include "../ui/mainwindow.h"
#include "../ui/optioncyclebutton.h"
#include "../components/slider/slidercomponent.h"
#include "../components/numeric/numericcomponent.h"

int runPhase3ModbusReadTests(int argc, char *argv[]);
int runPhase4ModbusWriteTests(int argc, char *argv[]);

class Phase1DataSourceTest : public QObject
{
    Q_OBJECT

private slots:
    void variable_defaultsAreReasonable();
    void variableModel_displaysMappingColumns();
    void addVariableFlow_writesToVariableModel();
    void duplicateVariableId_isRejected();
    void editVariableFlow_updatesModelImmediately();
    void deleteVariableFlow_removesRowSafely();
    void phase1Constraints_stillHold();
    void mappingButtons_areRemovedOrHiddenDisabled();
    void dataSourceButtons_areVisibleAndStateIsCorrect();
    void dataSourceTree_doesNotContainMappingsNode();
    void applySerialConfigFromPanel_updatesSerialDataSourceConfig();
    void slider_stepProperty_quantizesValue();
    void numeric_precisionProperty_updatesDecimals();
};

namespace {
void autoCloseMessageBox()
{
    QTimer::singleShot(0, []() {
        const auto widgets = QApplication::topLevelWidgets();
        for (QWidget *w : widgets) {
            if (auto *mb = qobject_cast<QMessageBox *>(w))
                mb->accept();
        }
    });
}
}

void Phase1DataSourceTest::variable_defaultsAreReasonable()
{
    Variable v;
    QCOMPARE(v.scale, 1.0);
    QCOMPARE(v.readOnly, true);
    QVERIFY(v.address >= 0);
    QVERIFY(v.count >= 1);
    QVERIFY(!v.type.isEmpty());
}

void Phase1DataSourceTest::variableModel_displaysMappingColumns()
{
    VariableModel model;
    Variable var;
    var.id = "V100";
    var.name = "Temp";
    var.deviceId = "RTU1";
    var.type = "float32";
    var.area = RegisterArea::HoldingRegister;
    var.address = 10;
    var.count = 2;
    var.value = 12.3;
    var.unit = "C";
    model.addVariable(var);

    QCOMPARE(model.rowCount({}), 1);
    QCOMPARE(model.data(model.index(0, VariableModel::ColId), Qt::DisplayRole).toString(), QString("V100"));
    QCOMPARE(model.data(model.index(0, VariableModel::ColName), Qt::DisplayRole).toString(), QString("Temp"));
    QCOMPARE(model.data(model.index(0, VariableModel::ColDevice), Qt::DisplayRole).toString(), QString("RTU1"));
    QCOMPARE(model.data(model.index(0, VariableModel::ColType), Qt::DisplayRole).toString(), QString("float32"));
    QCOMPARE(model.data(model.index(0, VariableModel::ColArea), Qt::DisplayRole).toString(), QString("HoldingRegister"));
    QCOMPARE(model.data(model.index(0, VariableModel::ColAddress), Qt::DisplayRole).toInt(), 10);
    QCOMPARE(model.data(model.index(0, VariableModel::ColCount), Qt::DisplayRole).toInt(), 2);
    QCOMPARE(model.data(model.index(0, VariableModel::ColValue), Qt::DisplayRole).toDouble(), 12.3);
    QCOMPARE(model.data(model.index(0, VariableModel::ColUnit), Qt::DisplayRole).toString(), QString("C"));

    QCOMPARE(model.headerData(VariableModel::ColId, Qt::Horizontal, Qt::DisplayRole).toString(), QString("ID"));
    QCOMPARE(model.headerData(VariableModel::ColName, Qt::Horizontal, Qt::DisplayRole).toString(), QString("Name"));
    QCOMPARE(model.headerData(VariableModel::ColDevice, Qt::Horizontal, Qt::DisplayRole).toString(), QString("Device"));
    QCOMPARE(model.headerData(VariableModel::ColType, Qt::Horizontal, Qt::DisplayRole).toString(), QString("Type"));
    QCOMPARE(model.headerData(VariableModel::ColArea, Qt::Horizontal, Qt::DisplayRole).toString(), QString("Area"));
    QCOMPARE(model.headerData(VariableModel::ColAddress, Qt::Horizontal, Qt::DisplayRole).toString(), QString("Address"));
    QCOMPARE(model.headerData(VariableModel::ColCount, Qt::Horizontal, Qt::DisplayRole).toString(), QString("Count"));
    QCOMPARE(model.headerData(VariableModel::ColValue, Qt::Horizontal, Qt::DisplayRole).toString(), QString("Value"));
    QCOMPARE(model.headerData(VariableModel::ColUnit, Qt::Horizontal, Qt::DisplayRole).toString(), QString("Unit"));
}

void Phase1DataSourceTest::addVariableFlow_writesToVariableModel()
{
    MainWindow window;
    window.show();
    QCoreApplication::processEvents();

    auto *addBtn = window.findChild<QPushButton *>("addVariableButton");
    QVERIFY(addBtn);
    addBtn->click();
    QCoreApplication::processEvents();

    window.findChild<QLineEdit *>("variableIdEdit")->setText("Temp01");
    window.findChild<QLineEdit *>("variableNameEdit")->setText("Temp");
    window.findChild<QLineEdit *>("variableDeviceEdit")->setText("Dev1");
    window.findChild<OptionCycleButton *>("variableTypeCombo")->setCurrentText("float32");
    window.findChild<OptionCycleButton *>("variableAreaCombo")->setCurrentText("HoldingRegister");
    window.findChild<QSpinBox *>("variableAddressSpin")->setValue(100);
    window.findChild<QSpinBox *>("variableCountSpin")->setValue(2);
    window.findChild<QSpinBox *>("variableBitOffsetSpin")->setValue(0);
    window.findChild<QLineEdit *>("variableUnitEdit")->setText("C");
    window.findChild<QDoubleSpinBox *>("variableScaleSpin")->setValue(1.5);
    window.findChild<QCheckBox *>("variableReadOnlyCheck")->setChecked(true);
    window.findChild<OptionCycleButton *>("variableEndianCombo")->setCurrentText("BigEndian");
    window.findChild<QPushButton *>("variableApplyButton")->click();
    QCoreApplication::processEvents();

    VariableModel *model = window.variableModelForTest();
    QVERIFY(model);
    QVERIFY(model->rowById("Temp01") >= 0);
    const int row = model->rowById("Temp01");
    QCOMPARE(model->data(model->index(row, VariableModel::ColAddress), Qt::DisplayRole).toInt(), 100);
    QCOMPARE(model->data(model->index(row, VariableModel::ColType), Qt::DisplayRole).toString(), QString("float32"));
    QCOMPARE(model->data(model->index(row, VariableModel::ColArea), Qt::DisplayRole).toString(), QString("HoldingRegister"));
}

void Phase1DataSourceTest::duplicateVariableId_isRejected()
{
    MainWindow window;
    window.show();
    QCoreApplication::processEvents();

    auto *model = window.variableModelForTest();
    QVERIFY(model);
    const int baseline = model->rowCount({});

    auto addOne = [&window](const QString &id) {
        window.findChild<QPushButton *>("addVariableButton")->click();
        QCoreApplication::processEvents();
        window.findChild<QLineEdit *>("variableIdEdit")->setText(id);
        window.findChild<QLineEdit *>("variableNameEdit")->setText("Name");
        window.findChild<QLineEdit *>("variableDeviceEdit")->setText("D");
        window.findChild<QPushButton *>("variableApplyButton")->click();
        QCoreApplication::processEvents();
    };
    addOne("Temp01");
    const int afterFirst = model->rowCount({});
    QVERIFY(afterFirst == baseline + 1 || model->rowById("Temp01") >= 0);

    window.findChild<QPushButton *>("addVariableButton")->click();
    QCoreApplication::processEvents();
    window.findChild<QLineEdit *>("variableIdEdit")->setText("Temp01");
    window.findChild<QLineEdit *>("variableNameEdit")->setText("Dup");
    autoCloseMessageBox();
    window.findChild<QPushButton *>("variableApplyButton")->click();
    QCoreApplication::processEvents();

    QCOMPARE(model->rowCount({}), afterFirst);
}

void Phase1DataSourceTest::editVariableFlow_updatesModelImmediately()
{
    MainWindow window;
    window.show();
    QCoreApplication::processEvents();
    auto *model = window.variableModelForTest();
    QVERIFY(model);

    Variable var;
    var.id = "Edit01";
    var.name = "Before";
    var.address = 1;
    var.count = 1;
    var.unit = "V";
    var.scale = 1.0;
    model->addVariable(var);
    const int row = model->rowById("Edit01");
    QVERIFY(row >= 0);

    auto *view = window.findChild<QTableView *>("variableView");
    QVERIFY(view);
    view->selectRow(row);
    QCoreApplication::processEvents();

    window.findChild<QPushButton *>("editVariableButton")->click();
    QCoreApplication::processEvents();
    window.findChild<QSpinBox *>("variableAddressSpin")->setValue(88);
    window.findChild<QSpinBox *>("variableCountSpin")->setValue(3);
    window.findChild<QLineEdit *>("variableUnitEdit")->setText("bar");
    window.findChild<QDoubleSpinBox *>("variableScaleSpin")->setValue(2.0);
    window.findChild<QPushButton *>("variableApplyButton")->click();
    QCoreApplication::processEvents();

    QCOMPARE(model->data(model->index(row, VariableModel::ColAddress), Qt::DisplayRole).toInt(), 88);
    QCOMPARE(model->data(model->index(row, VariableModel::ColCount), Qt::DisplayRole).toInt(), 3);
    QCOMPARE(model->data(model->index(row, VariableModel::ColUnit), Qt::DisplayRole).toString(), QString("bar"));
    QCOMPARE(model->data(model->index(row, VariableModel::ColScale), Qt::DisplayRole).toDouble(), 2.0);
}

void Phase1DataSourceTest::deleteVariableFlow_removesRowSafely()
{
    MainWindow window;
    window.show();
    QCoreApplication::processEvents();
    auto *model = window.variableModelForTest();
    QVERIFY(model);

    Variable a; a.id = "DelA"; a.name = "A"; model->addVariable(a);
    Variable b; b.id = "DelB"; b.name = "B"; model->addVariable(b);
    const int row = model->rowById("DelA");
    QVERIFY(row >= 0);
    const int before = model->rowCount({});

    auto *view = window.findChild<QTableView *>("variableView");
    QVERIFY(view);
    view->selectRow(row);
    QCoreApplication::processEvents();
    window.findChild<QPushButton *>("deleteVariableButton")->click();
    QCoreApplication::processEvents();

    QCOMPARE(model->rowCount({}), before - 1);
    QCOMPARE(model->rowById("DelA"), -1);
    QVERIFY(!view->selectionModel()->currentIndex().isValid() || view->selectionModel()->currentIndex().row() < model->rowCount({}));
}

void Phase1DataSourceTest::phase1Constraints_stillHold()
{
    mappingButtons_areRemovedOrHiddenDisabled();
    dataSourceTree_doesNotContainMappingsNode();
}

void Phase1DataSourceTest::mappingButtons_areRemovedOrHiddenDisabled()
{
    MainWindow window;
    window.show();
    QCoreApplication::processEvents();

    const QStringList mappingButtonNames = {
        "mappingAddButton",
        "mappingDeleteButton",
        "mappingEditButton"
    };

    for (const QString &name : mappingButtonNames) {
        auto *button = window.findChild<QPushButton *>(name);
        if (!button)
            continue;
        QVERIFY(!button->isVisible());
        QVERIFY(!button->isEnabled());
    }
}

void Phase1DataSourceTest::dataSourceButtons_areVisibleAndStateIsCorrect()
{
    MainWindow window;
    window.show();
    QCoreApplication::processEvents();

    auto *configButton = window.findChild<QPushButton *>("dataSourceConfigButton");
    auto *removeButton = window.findChild<QPushButton *>("dataSourceRemoveButton");
    auto *openButton = window.findChild<QPushButton *>("dataSourceOpenButton");
    auto *closeButton = window.findChild<QPushButton *>("dataSourceCloseButton");

    QVERIFY(configButton);
    QVERIFY(removeButton);
    QVERIFY(openButton);
    QVERIFY(closeButton);

    QVERIFY(configButton->isEnabled());
    QVERIFY(removeButton->isEnabled());
    QVERIFY(openButton->isEnabled());
    QVERIFY(!closeButton->isEnabled());
}

namespace {
void collectNodeTexts(const QAbstractItemModel *model, const QModelIndex &parent, QStringList *out)
{
    if (!model || !out)
        return;

    for (int row = 0; row < model->rowCount(parent); ++row) {
        const QModelIndex index = model->index(row, 0, parent);
        out->append(index.data().toString());
        collectNodeTexts(model, index, out);
    }
}
}

void Phase1DataSourceTest::dataSourceTree_doesNotContainMappingsNode()
{
    MainWindow window;
    window.show();
    QCoreApplication::processEvents();

    auto *tree = window.findChild<QTreeView *>("treeView");
    QVERIFY(tree);
    QVERIFY(tree->model());

    QStringList nodeTexts;
    collectNodeTexts(tree->model(), QModelIndex(), &nodeTexts);
    QVERIFY(!nodeTexts.contains("Mappings"));
}

void Phase1DataSourceTest::applySerialConfigFromPanel_updatesSerialDataSourceConfig()
{
    MainWindow window;
    window.show();
    QCoreApplication::processEvents();

    QVERIFY(QMetaObject::invokeMethod(&window, "showSerialConfigDialog", Qt::DirectConnection));

    auto *portEdit = window.findChild<QLineEdit *>("serialPortEdit");
    auto *baudCombo = window.findChild<OptionCycleButton *>("serialBaudCombo");
    auto *slaveIdSpin = window.findChild<QSpinBox *>("serialSlaveIdSpin");
    auto *timeoutSpin = window.findChild<QSpinBox *>("serialTimeoutSpin");

    QVERIFY(portEdit);
    QVERIFY(baudCombo);
    QVERIFY(slaveIdSpin);
    QVERIFY(timeoutSpin);

    portEdit->setText("COM9");
    baudCombo->setCurrentText("19200");
    slaveIdSpin->setValue(7);
    timeoutSpin->setValue(2200);

    QVERIFY(QMetaObject::invokeMethod(&window, "applySerialConfigFromPanel", Qt::DirectConnection));

    SerialDataSource *source = window.serialDataSourceForTest();
    QVERIFY(source);
    const SerialPortConfig cfg = source->config();
    QCOMPARE(cfg.portName, QString("COM9"));
    QCOMPARE(cfg.baudRate, 19200);
    QCOMPARE(cfg.slaveId, 7);
    QCOMPARE(cfg.timeoutMs, 2200);
}

void Phase1DataSourceTest::slider_stepProperty_quantizesValue()
{
    SliderComponent slider;
    slider.setPropertyValue("step", 2.0);
    slider.setPropertyValue("value", 13.0);
    QCOMPARE(slider.properties().value("value").toDouble(), 14.0);
}

void Phase1DataSourceTest::numeric_precisionProperty_updatesDecimals()
{
    NumericComponent numeric;
    numeric.setPropertyValue("precision", 3);
    numeric.setPropertyValue("value", 12.34567);
    QCOMPARE(numeric.properties().value("precision").toInt(), 3);
    QCOMPARE(numeric.properties().value("decimals").toInt(), 3);
}

int main(int argc, char *argv[])
{
    qputenv("QT_QPA_PLATFORM", QByteArray("offscreen"));
    QApplication app(argc, argv);
    Phase1DataSourceTest tc;
    const int r1 = QTest::qExec(&tc, argc, argv);
    const int r2 = runPhase3ModbusReadTests(argc, argv);
    const int r3 = runPhase4ModbusWriteTests(argc, argv);
    return r1 == 0 && r2 == 0 && r3 == 0 ? 0 : 1;
}

#include "tst_phase1_datasource.moc"
