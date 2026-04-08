#ifndef SERIALDATASOURCE_H
#define SERIALDATASOURCE_H

#pragma once

#include <QObject>
#include <QByteArray>
#include <QHash>
#include <QSerialPort>

class VariableModel;

struct SerialPortConfig {
    QString deviceId = QStringLiteral("default");
    QString portName = QStringLiteral("/dev/ttyS2");
    qint32 baudRate = QSerialPort::Baud9600;
    QSerialPort::DataBits dataBits = QSerialPort::Data8;
    QSerialPort::Parity parity = QSerialPort::NoParity;
    QSerialPort::StopBits stopBits = QSerialPort::OneStop;
    QSerialPort::FlowControl flowControl = QSerialPort::NoFlowControl;
    int slaveId = 1;
    int timeoutMs = 1000;
    int retryCount = 3;
    int pollIntervalMs = 500;
    int defaultFunctionCode = 3;
    QByteArray frameTerminator = "\n";
};

class SerialDataSource : public QObject
{
    Q_OBJECT
public:
    explicit SerialDataSource(QObject *parent = nullptr);

    void setConfig(const SerialPortConfig &config);
    SerialPortConfig config() const { return m_config; }

    bool open();
    void close();
    bool isOpen() const;

signals:
    void frameReceived(const QByteArray &frame);
    void rawBytesReceived(const QByteArray &bytes);
    void statusChanged(bool opened);
    void errorOccurred(const QString &errorText);

private slots:
    void onReadyRead();
    void onSerialError(QSerialPort::SerialPortError err);

private:
    void applyPortConfig();
    void extractFrames();

    QSerialPort m_serial;
    SerialPortConfig m_config;
    QByteArray m_rxBuffer;
};

class SerialVariableMapper : public QObject
{
    Q_OBJECT
public:
    explicit SerialVariableMapper(VariableModel *model, QObject *parent = nullptr);

    void setBinding(const QString &sourceKey, const QString &varId);
    void removeBinding(const QString &sourceKey);
    void clearBindings();
    QHash<QString, QString> bindings() const { return m_keyToVarId; }

public slots:
    void onFrameReceived(const QByteArray &frame);

private:
    void processKeyValue(const QString &key, const QString &rawValue);

    VariableModel *m_model = nullptr;
    QHash<QString, QString> m_keyToVarId;
};

#endif // SERIALDATASOURCE_H
