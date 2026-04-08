#ifndef MODBUSRTUDATASOURCE_H
#define MODBUSRTUDATASOURCE_H

#include <QObject>
#include <QTimer>
#include <QSerialPort>
#include <QVariant>

#include "serialdatasource.h"
#include "model/variablemodel.h"

class ModbusRtuDataSource : public QObject
{
    Q_OBJECT
public:
    explicit ModbusRtuDataSource(QObject *parent = nullptr);

    void setConfig(const SerialPortConfig &config);
    SerialPortConfig config() const { return m_config; }

    void setVariableModel(VariableModel *model);
    bool open();
    void close();
    bool isOpen() const;

    void startPolling();
    void stopPolling();
    bool isPolling() const { return m_polling; }

    QVector<int> eligibleVariableRowsForTest() const;
    bool processReadResultForTest(const QString &varId, const QVector<quint16> &registers, QString *error = nullptr);
    static QVariant decodeRegisters(const Variable &var, const QVector<quint16> &registers, bool *ok, QString *errorText = nullptr);

signals:
    void statusChanged(bool opened);
    void pollingStateChanged(bool running);
    void errorOccurred(const QString &errorText);
    void variableReadSucceeded(const QString &varId, const QVariant &value);
    void variableReadFailed(const QString &varId, const QString &reason);

private slots:
    void pollNextVariable();

private:
    static quint16 crc16(const QByteArray &payload);
    bool readVariableAtRow(int row);
    bool parseResponse(const Variable &var, quint8 functionCode, const QByteArray &resp, QVector<quint16> *outRegs, QString *errorText) const;

    QSerialPort m_serial;
    QTimer m_pollTimer;
    SerialPortConfig m_config;
    VariableModel *m_model = nullptr;
    bool m_polling = false;
    int m_nextRowCursor = 0;
};

#endif // MODBUSRTUDATASOURCE_H
