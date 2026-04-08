#include <QtTest>

#include <QAbstractItemModel>
#include <QApplication>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTreeView>

#include "../ui/mainwindow.h"

class Phase1DataSourceTest : public QObject
{
    Q_OBJECT

private slots:
    void mappingButtons_areRemovedOrHiddenDisabled();
    void dataSourceButtons_areVisibleAndStateIsCorrect();
    void dataSourceTree_doesNotContainMappingsNode();
    void applySerialConfigFromPanel_updatesSerialDataSourceConfig();
};

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
    auto *baudCombo = window.findChild<QComboBox *>("serialBaudCombo");
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

int main(int argc, char *argv[])
{
    qputenv("QT_QPA_PLATFORM", QByteArray("offscreen"));
    QApplication app(argc, argv);
    Phase1DataSourceTest tc;
    return QTest::qExec(&tc, argc, argv);
}

#include "tst_phase1_datasource.moc"
