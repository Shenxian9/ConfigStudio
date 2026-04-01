#include "serialdatasource.h"
#include "../model/variablemodel.h"

#include <QStringList>

SerialDataSource::SerialDataSource(QObject *parent)
    : QObject(parent)
{
    connect(&m_serial, &QSerialPort::readyRead,
            this, &SerialDataSource::onReadyRead);
    connect(&m_serial, &QSerialPort::errorOccurred,
            this, &SerialDataSource::onSerialError);
}

void SerialDataSource::setConfig(const SerialPortConfig &config)
{
    const bool wasOpen = m_serial.isOpen();
    if (wasOpen)
        close();

    m_config = config;

    if (wasOpen)
        open();
}

bool SerialDataSource::open()
{
    if (m_serial.isOpen())
        return true;

    m_serial.setPortName(m_config.portName);
    applyPortConfig();

    if (!m_serial.open(QIODevice::ReadWrite)) {
        emit errorOccurred(m_serial.errorString());
        return false;
    }

    m_rxBuffer.clear();
    emit statusChanged(true);
    return true;
}

void SerialDataSource::close()
{
    if (!m_serial.isOpen())
        return;

    m_serial.close();
    m_rxBuffer.clear();
    emit statusChanged(false);
}

bool SerialDataSource::isOpen() const
{
    return m_serial.isOpen();
}

void SerialDataSource::onReadyRead()
{
    const QByteArray bytes = m_serial.readAll();
    if (bytes.isEmpty())
        return;

    m_rxBuffer.append(bytes);
    emit rawBytesReceived(bytes);
    extractFrames();
}

void SerialDataSource::onSerialError(QSerialPort::SerialPortError err)
{
    if (err == QSerialPort::NoError)
        return;

    emit errorOccurred(m_serial.errorString());
}

void SerialDataSource::applyPortConfig()
{
    m_serial.setBaudRate(m_config.baudRate);
    m_serial.setDataBits(m_config.dataBits);
    m_serial.setParity(m_config.parity);
    m_serial.setStopBits(m_config.stopBits);
    m_serial.setFlowControl(m_config.flowControl);
}

void SerialDataSource::extractFrames()
{
    if (m_config.frameTerminator.isEmpty()) {
        if (!m_rxBuffer.isEmpty()) {
            emit frameReceived(m_rxBuffer);
            m_rxBuffer.clear();
        }
        return;
    }

    while (true) {
        const int splitPos = m_rxBuffer.indexOf(m_config.frameTerminator);
        if (splitPos < 0)
            break;

        QByteArray frame = m_rxBuffer.left(splitPos);
        frame = frame.trimmed();
        m_rxBuffer.remove(0, splitPos + m_config.frameTerminator.size());

        if (!frame.isEmpty())
            emit frameReceived(frame);
    }
}

SerialVariableMapper::SerialVariableMapper(VariableModel *model, QObject *parent)
    : QObject(parent)
    , m_model(model)
{
}

void SerialVariableMapper::setBinding(const QString &sourceKey, const QString &varId)
{
    const QString key = sourceKey.trimmed();
    const QString id = varId.trimmed();

    if (key.isEmpty() || id.isEmpty())
        return;

    m_keyToVarId.insert(key, id);
}

void SerialVariableMapper::removeBinding(const QString &sourceKey)
{
    m_keyToVarId.remove(sourceKey.trimmed());
}

void SerialVariableMapper::clearBindings()
{
    m_keyToVarId.clear();
}

void SerialVariableMapper::onFrameReceived(const QByteArray &frame)
{
    const QString text = QString::fromUtf8(frame).trimmed();
    if (text.isEmpty())
        return;

    QString normalized = text;
    normalized.replace(',', ';');
    const QStringList segments = normalized.split(';', Qt::SkipEmptyParts);
    for (const QString &segment : segments) {
        const int eq = segment.indexOf('=');
        if (eq <= 0)
            continue;

        const QString key = segment.left(eq).trimmed();
        const QString rawValue = segment.mid(eq + 1).trimmed();
        processKeyValue(key, rawValue);
    }
}

void SerialVariableMapper::processKeyValue(const QString &key, const QString &rawValue)
{
    if (!m_model)
        return;

    const QString varId = m_keyToVarId.value(key);
    if (varId.isEmpty())
        return;

    bool ok = false;
    const double numValue = rawValue.toDouble(&ok);

    if (ok)
        m_model->updateValueById(varId, numValue);
    else
        m_model->updateValueById(varId, rawValue);
}
