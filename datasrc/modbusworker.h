#ifndef MODBUSWORKER_H
#define MODBUSWORKER_H

#pragma once

#include <QObject>
#include <QByteArray>
#include <QQueue>

#include "serialdatasource.h"

enum class ModbusRequestType {
    TestRawRequest,
};

struct ModbusRequest {
    QString requestId;
    ModbusRequestType type = ModbusRequestType::TestRawRequest;
    int slaveId = 1;
    int functionCode = 3;
    QByteArray pduOrPayload;
    int timeoutMs = 1000;
    int retryCount = 0;
    QString description;
};
Q_DECLARE_METATYPE(ModbusRequest)

struct ModbusResponse {
    QString requestId;
    bool success = false;
    QByteArray requestFrame;
    QByteArray responseFrame;
    QString errorText;
    int elapsedMs = 0;
};
Q_DECLARE_METATYPE(ModbusResponse)

class QElapsedTimer;
class QSerialPort;
class QTimer;

class ModbusWorker : public QObject
{
    Q_OBJECT
public:
    explicit ModbusWorker(QObject *parent = nullptr);

public slots:
    void initialize();
    void setConnectionConfig(const SerialPortConfig &cfg);
    void openPort();
    void closePort();
    void enqueueRequest(const ModbusRequest &request);

signals:
    void responseReady(const ModbusResponse &response);
    void portStatusChanged(bool opened);
    void workerLog(const QString &msg);

private slots:
    void onReadyRead();
    void onTimeout();
    void onSerialError(QSerialPort::SerialPortError error);

private:
    struct PendingRequest {
        ModbusRequest req;
        int retriesLeft = 0;
        QByteArray txFrame;
    };

    void tryProcessNext();
    void startCurrentRequest();
    void completeCurrent(bool success, const QString &errorText = QString());
    bool isMockMode() const;

    SerialPortConfig m_config;
    QSerialPort *m_serial = nullptr;
    QTimer *m_timeoutTimer = nullptr;
    QQueue<ModbusRequest> m_queue;
    bool m_busy = false;
    PendingRequest m_current;
    QByteArray m_rxBuffer;
    QElapsedTimer *m_elapsed = nullptr;
};

class QThread;
class ModbusController : public QObject
{
    Q_OBJECT
public:
    explicit ModbusController(QObject *parent = nullptr);
    ~ModbusController() override;

    void applyConnectionConfig(const SerialPortConfig &cfg);
    void openPort();
    void closePort();
    void enqueueRequest(const ModbusRequest &request);

signals:
    void requestSetConfig(const SerialPortConfig &cfg);
    void requestOpenPort();
    void requestClosePort();
    void requestEnqueue(const ModbusRequest &request);

    void responseReady(const ModbusResponse &response);
    void portStatusChanged(bool opened);
    void workerLog(const QString &msg);

private:
    QThread *m_thread = nullptr;
    ModbusWorker *m_worker = nullptr;
};

#endif // MODBUSWORKER_H
