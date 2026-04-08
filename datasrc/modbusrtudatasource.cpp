#include "modbusrtudatasource.h"

#include <QtEndian>
#include <QDataStream>
#include <cstring>

ModbusRtuDataSource::ModbusRtuDataSource(QObject *parent)
    : QObject(parent)
{
    connect(&m_pollTimer, &QTimer::timeout, this, &ModbusRtuDataSource::pollNextVariable);
}

void ModbusRtuDataSource::setConfig(const SerialPortConfig &config)
{
    m_config = config;
    if (m_polling)
        m_pollTimer.start(qMax(50, m_config.pollIntervalMs));
}

void ModbusRtuDataSource::setVariableModel(VariableModel *model)
{
    m_model = model;
}

bool ModbusRtuDataSource::open()
{
    if (m_serial.isOpen())
        return true;

    m_serial.setPortName(m_config.portName);
    m_serial.setBaudRate(m_config.baudRate);
    m_serial.setDataBits(m_config.dataBits);
    m_serial.setParity(m_config.parity);
    m_serial.setStopBits(m_config.stopBits);
    m_serial.setFlowControl(m_config.flowControl);

    if (!m_serial.open(QIODevice::ReadWrite)) {
        emit errorOccurred(tr("Open serial failed: %1").arg(m_serial.errorString()));
        emit statusChanged(false);
        return false;
    }

    emit statusChanged(true);
    return true;
}

void ModbusRtuDataSource::close()
{
    stopPolling();
    if (m_serial.isOpen())
        m_serial.close();
    emit statusChanged(false);
}

bool ModbusRtuDataSource::isOpen() const
{
    return m_serial.isOpen();
}

void ModbusRtuDataSource::startPolling()
{
    if (!m_serial.isOpen() || !m_model)
        return;
    if (m_polling)
        return;
    m_polling = true;
    m_nextRowCursor = 0;
    m_pollTimer.start(qMax(50, m_config.pollIntervalMs));
    emit pollingStateChanged(true);
}

void ModbusRtuDataSource::stopPolling()
{
    if (!m_polling)
        return;
    m_polling = false;
    m_pollTimer.stop();
    emit pollingStateChanged(false);
}

QVector<int> ModbusRtuDataSource::eligibleVariableRowsForTest() const
{
    QVector<int> rows;
    if (!m_model)
        return rows;

    for (int i = 0; i < m_model->rowCount({}); ++i) {
        const Variable &var = m_model->variableAt(i);
        if (!m_config.deviceId.trimmed().isEmpty() && var.deviceId != m_config.deviceId)
            continue;
        if (!(var.area == RegisterArea::HoldingRegister || var.area == RegisterArea::InputRegister))
            continue;
        if (var.address < 0 || var.count < 1)
            continue;
        const QString t = var.type.trimmed().toLower();
        if (t != "uint16" && t != "int16" && t != "uint32" && t != "int32" && t != "float32" && t != "bool")
            continue;
        rows.append(i);
    }
    return rows;
}

bool ModbusRtuDataSource::processReadResultForTest(const QString &varId, const QVector<quint16> &registers, QString *error)
{
    if (!m_model) {
        if (error) *error = "model is null";
        return false;
    }
    const int row = m_model->rowById(varId);
    if (row < 0) {
        if (error) *error = "varId not found";
        return false;
    }
    const Variable &var = m_model->variableAt(row);
    bool ok = false;
    QString err;
    const QVariant decoded = decodeRegisters(var, registers, &ok, &err);
    if (!ok) {
        if (error) *error = err;
        emit variableReadFailed(varId, err);
        return false;
    }
    m_model->updateValueById(varId, decoded);
    emit variableReadSucceeded(varId, decoded);
    return true;
}

bool ModbusRtuDataSource::encodeSingleRegisterWriteForTest(const Variable &var, const QVariant &value, quint16 *encoded, QString *errorText) const
{
    return encodeSingleRegisterWrite(var, value, encoded, errorText);
}

bool ModbusRtuDataSource::encodeSingleRegisterWrite(const Variable &var, const QVariant &value, quint16 *encoded, QString *errorText) const
{
    if (!encoded)
        return false;
    const QString t = var.type.trimmed().toLower();
    if (var.area != RegisterArea::HoldingRegister) {
        if (errorText) *errorText = "only HoldingRegister write is supported";
        return false;
    }
    if (var.readOnly) {
        if (errorText) *errorText = "variable is readOnly";
        return false;
    }
    if (var.address < 0 || var.count != 1) {
        if (errorText) *errorText = "only single-register write is supported";
        return false;
    }
    if (t == "bool") {
        if (var.bitOffset != 0) {
            if (errorText) *errorText = "bit-level register write is not supported in phase4";
            return false;
        }
        *encoded = value.toBool() ? 1 : 0;
        return true;
    }
    if (t == "uint16") {
        bool ok = false;
        const uint v = value.toUInt(&ok);
        if (!ok || v > 0xFFFFu) {
            if (errorText) *errorText = "uint16 value out of range";
            return false;
        }
        *encoded = static_cast<quint16>(v);
        return true;
    }
    if (t == "int16") {
        bool ok = false;
        const int v = value.toInt(&ok);
        if (!ok || v < -32768 || v > 32767) {
            if (errorText) *errorText = "int16 value out of range";
            return false;
        }
        *encoded = static_cast<quint16>(static_cast<qint16>(v));
        return true;
    }
    if (errorText) *errorText = "unsupported type for single-register write";
    return false;
}

bool ModbusRtuDataSource::writeVariable(const QString &varId, const QVariant &value, QString *errorText)
{
    if (!m_writeEnabled) {
        if (errorText) *errorText = "write disabled";
        return false;
    }
    if (!m_model) {
        if (errorText) *errorText = "model is null";
        return false;
    }
    if (!m_serial.isOpen()) {
        if (errorText) *errorText = "serial is not open";
        return false;
    }
    const int row = m_model->rowById(varId);
    if (row < 0) {
        if (errorText) *errorText = "varId not found";
        return false;
    }
    const Variable &var = m_model->variableAt(row);
    quint16 encoded = 0;
    QString encodeErr;
    if (!encodeSingleRegisterWrite(var, value, &encoded, &encodeErr)) {
        if (errorText) *errorText = encodeErr;
        emit variableWriteFailed(varId, encodeErr);
        return false;
    }

    const quint16 address = static_cast<quint16>(var.address);
    QByteArray req;
    req.append(static_cast<char>(m_config.slaveId));
    req.append(static_cast<char>(0x06));
    req.append(static_cast<char>((address >> 8) & 0xFF));
    req.append(static_cast<char>(address & 0xFF));
    req.append(static_cast<char>((encoded >> 8) & 0xFF));
    req.append(static_cast<char>(encoded & 0xFF));
    const quint16 crc = crc16(req);
    req.append(static_cast<char>(crc & 0xFF));
    req.append(static_cast<char>((crc >> 8) & 0xFF));

    if (m_serial.write(req) != req.size() || !m_serial.waitForBytesWritten(m_config.timeoutMs)) {
        const QString err = tr("write request failed: %1").arg(m_serial.errorString());
        if (errorText) *errorText = err;
        emit variableWriteFailed(varId, err);
        emit errorOccurred(err);
        return false;
    }
    if (!m_serial.waitForReadyRead(m_config.timeoutMs)) {
        const QString err = tr("write response timeout");
        if (errorText) *errorText = err;
        emit variableWriteFailed(varId, err);
        return false;
    }
    QByteArray resp = m_serial.readAll();
    while (m_serial.waitForReadyRead(10))
        resp += m_serial.readAll();

    if (resp.size() < 8) {
        const QString err = tr("write response too short");
        if (errorText) *errorText = err;
        emit variableWriteFailed(varId, err);
        return false;
    }
    const QByteArray payload = resp.left(resp.size() - 2);
    const quint16 recvCrc = static_cast<quint8>(resp.at(resp.size() - 2))
                            | (static_cast<quint16>(static_cast<quint8>(resp.at(resp.size() - 1))) << 8);
    if (crc16(payload) != recvCrc || payload.left(6) != req.left(6)) {
        const QString err = tr("write response validation failed");
        if (errorText) *errorText = err;
        emit variableWriteFailed(varId, err);
        return false;
    }
    emit variableWriteSucceeded(varId);
    return true;
}

QVariant ModbusRtuDataSource::decodeRegisters(const Variable &var, const QVector<quint16> &registers, bool *ok, QString *errorText)
{
    auto fail = [&](const QString &msg) -> QVariant {
        if (ok) *ok = false;
        if (errorText) *errorText = msg;
        return {};
    };

    const QString type = var.type.trimmed().toLower();
    if (registers.isEmpty())
        return fail("empty register list");

    if (type == "bool") {
        const int bit = qBound(0, var.bitOffset, 15);
        const bool v = ((registers.first() >> bit) & 0x1) != 0;
        if (ok) *ok = true;
        return v;
    }

    auto applyScale = [&](double v) { return v * (var.scale <= 0.0 ? 1.0 : var.scale); };

    if (type == "uint16") {
        if (ok) *ok = true;
        return applyScale(static_cast<quint16>(registers.first()));
    }
    if (type == "int16") {
        if (ok) *ok = true;
        return applyScale(static_cast<qint16>(registers.first()));
    }

    if (registers.size() < 2)
        return fail("count < 2 for 32-bit type");

    const quint16 r0 = registers[0];
    const quint16 r1 = registers[1];
    quint8 b0 = static_cast<quint8>((r0 >> 8) & 0xFF);
    quint8 b1 = static_cast<quint8>(r0 & 0xFF);
    quint8 b2 = static_cast<quint8>((r1 >> 8) & 0xFF);
    quint8 b3 = static_cast<quint8>(r1 & 0xFF);

    switch (var.endianness) {
    case Endianness::BigEndianWordSwap:
        b0 = static_cast<quint8>((r1 >> 8) & 0xFF);
        b1 = static_cast<quint8>(r1 & 0xFF);
        b2 = static_cast<quint8>((r0 >> 8) & 0xFF);
        b3 = static_cast<quint8>(r0 & 0xFF);
        break;
    case Endianness::LittleEndian:
        b0 = static_cast<quint8>(r1 & 0xFF);
        b1 = static_cast<quint8>((r1 >> 8) & 0xFF);
        b2 = static_cast<quint8>(r0 & 0xFF);
        b3 = static_cast<quint8>((r0 >> 8) & 0xFF);
        break;
    case Endianness::LittleEndianByteSwap:
        b0 = static_cast<quint8>(r0 & 0xFF);
        b1 = static_cast<quint8>((r0 >> 8) & 0xFF);
        b2 = static_cast<quint8>(r1 & 0xFF);
        b3 = static_cast<quint8>((r1 >> 8) & 0xFF);
        break;
    case Endianness::BigEndian:
    default:
        break;
    }

    const quint32 raw = (static_cast<quint32>(b0) << 24)
                        | (static_cast<quint32>(b1) << 16)
                        | (static_cast<quint32>(b2) << 8)
                        | static_cast<quint32>(b3);

    if (type == "uint32") {
        if (ok) *ok = true;
        return applyScale(static_cast<double>(raw));
    }
    if (type == "int32") {
        if (ok) *ok = true;
        return applyScale(static_cast<double>(static_cast<qint32>(raw)));
    }
    if (type == "float32") {
        float f = 0.0f;
        std::memcpy(&f, &raw, sizeof(float));
        if (ok) *ok = true;
        return applyScale(static_cast<double>(f));
    }

    return fail("unsupported type");
}

void ModbusRtuDataSource::pollNextVariable()
{
    if (!m_polling || !m_serial.isOpen() || !m_model)
        return;

    const QVector<int> rows = eligibleVariableRowsForTest();
    if (rows.isEmpty())
        return;

    if (m_nextRowCursor >= rows.size())
        m_nextRowCursor = 0;
    const int row = rows.at(m_nextRowCursor++);
    readVariableAtRow(row);
}

quint16 ModbusRtuDataSource::crc16(const QByteArray &payload)
{
    quint16 crc = 0xFFFF;
    for (unsigned char b : payload) {
        crc ^= b;
        for (int i = 0; i < 8; ++i) {
            if (crc & 0x0001) crc = (crc >> 1) ^ 0xA001;
            else crc >>= 1;
        }
    }
    return crc;
}

bool ModbusRtuDataSource::readVariableAtRow(int row)
{
    if (!m_model || row < 0 || row >= m_model->rowCount({}))
        return false;

    const Variable &var = m_model->variableAt(row);
    const quint8 fc = (var.area == RegisterArea::HoldingRegister) ? 0x03 : 0x04;
    const quint16 address = static_cast<quint16>(var.address);
    const quint16 quantity = static_cast<quint16>(qMax(1, var.count));

    QByteArray req;
    req.append(static_cast<char>(m_config.slaveId));
    req.append(static_cast<char>(fc));
    req.append(static_cast<char>((address >> 8) & 0xFF));
    req.append(static_cast<char>(address & 0xFF));
    req.append(static_cast<char>((quantity >> 8) & 0xFF));
    req.append(static_cast<char>(quantity & 0xFF));
    const quint16 crc = crc16(req);
    req.append(static_cast<char>(crc & 0xFF));
    req.append(static_cast<char>((crc >> 8) & 0xFF));

    m_serial.clear(QSerialPort::AllDirections);
    if (m_serial.write(req) != req.size() || !m_serial.waitForBytesWritten(m_config.timeoutMs)) {
        const QString err = tr("write request failed: %1").arg(m_serial.errorString());
        emit variableReadFailed(var.id, err);
        emit errorOccurred(err);
        return false;
    }

    QByteArray resp;
    if (!m_serial.waitForReadyRead(m_config.timeoutMs)) {
        const QString err = tr("read timeout for %1").arg(var.id);
        emit variableReadFailed(var.id, err);
        return false;
    }
    resp += m_serial.readAll();
    while (m_serial.waitForReadyRead(10))
        resp += m_serial.readAll();

    QVector<quint16> regs;
    QString error;
    if (!parseResponse(var, fc, resp, &regs, &error)) {
        emit variableReadFailed(var.id, error);
        emit errorOccurred(error);
        return false;
    }

    bool ok = false;
    QString decodeError;
    const QVariant value = decodeRegisters(var, regs, &ok, &decodeError);
    if (!ok) {
        emit variableReadFailed(var.id, decodeError);
        return false;
    }
    m_model->updateValueById(var.id, value);
    emit variableReadSucceeded(var.id, value);
    return true;
}

bool ModbusRtuDataSource::parseResponse(const Variable &var, quint8 functionCode, const QByteArray &resp, QVector<quint16> *outRegs, QString *errorText) const
{
    if (!outRegs)
        return false;
    outRegs->clear();
    if (resp.size() < 5) {
        if (errorText) *errorText = tr("response too short for %1").arg(var.id);
        return false;
    }
    const quint16 receivedCrc = static_cast<quint8>(resp.at(resp.size() - 2))
                                | (static_cast<quint16>(static_cast<quint8>(resp.at(resp.size() - 1))) << 8);
    const QByteArray payload = resp.left(resp.size() - 2);
    if (crc16(payload) != receivedCrc) {
        if (errorText) *errorText = tr("crc mismatch for %1").arg(var.id);
        return false;
    }

    const quint8 addr = static_cast<quint8>(payload.at(0));
    Q_UNUSED(addr);
    const quint8 fc = static_cast<quint8>(payload.at(1));
    if (fc == (functionCode | 0x80)) {
        const quint8 exceptionCode = payload.size() > 2 ? static_cast<quint8>(payload.at(2)) : 0;
        if (errorText) *errorText = tr("modbus exception code %1").arg(static_cast<int>(exceptionCode));
        return false;
    }
    if (fc != functionCode) {
        if (errorText) *errorText = tr("unexpected function code");
        return false;
    }
    const int byteCount = static_cast<quint8>(payload.at(2));
    if (byteCount <= 0 || payload.size() != 3 + byteCount) {
        if (errorText) *errorText = tr("invalid byte count");
        return false;
    }
    if (byteCount % 2 != 0) {
        if (errorText) *errorText = tr("odd byte count");
        return false;
    }
    for (int i = 0; i < byteCount; i += 2) {
        const quint16 reg = (static_cast<quint16>(static_cast<quint8>(payload.at(3 + i))) << 8)
                            | static_cast<quint8>(payload.at(3 + i + 1));
        outRegs->append(reg);
    }
    return true;
}
