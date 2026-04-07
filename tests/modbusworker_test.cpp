#include <QtTest>
#include <QSignalSpy>

#include "../datasrc/modbusworker.h"

class ModbusWorkerTest : public QObject
{
    Q_OBJECT
private slots:
    void mockMode_requestsAreProcessedInOrder();
};

void ModbusWorkerTest::mockMode_requestsAreProcessedInOrder()
{
    ModbusController controller;

    QSignalSpy spy(&controller, &ModbusController::responseReady);

    SerialPortConfig cfg;
    cfg.portName = "mock";
    cfg.timeoutMs = 100;
    controller.applyConnectionConfig(cfg);
    controller.openPort();

    ModbusRequest r1; r1.requestId = "r1"; r1.pduOrPayload = "A";
    ModbusRequest r2; r2.requestId = "r2"; r2.pduOrPayload = "B";
    ModbusRequest r3; r3.requestId = "r3"; r3.pduOrPayload = "C";

    controller.enqueueRequest(r1);
    controller.enqueueRequest(r2);
    controller.enqueueRequest(r3);

    QTRY_COMPARE_WITH_TIMEOUT(spy.count(), 3, 2000);

    const ModbusResponse a = spy.at(0).at(0).value<ModbusResponse>();
    const ModbusResponse b = spy.at(1).at(0).value<ModbusResponse>();
    const ModbusResponse c = spy.at(2).at(0).value<ModbusResponse>();

    QCOMPARE(a.requestId, QString("r1"));
    QCOMPARE(b.requestId, QString("r2"));
    QCOMPARE(c.requestId, QString("r3"));
    QVERIFY(a.success);
    QVERIFY(b.success);
    QVERIFY(c.success);
}

QTEST_MAIN(ModbusWorkerTest)
#include "modbusworker_test.moc"
