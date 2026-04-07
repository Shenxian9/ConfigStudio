#include "singlegrouppollrunner.h"

#include <QDateTime>
#include <QDebug>
#include <QTimer>
#include <cstring>

#include "runtime/databindingmanager.h"

namespace {
quint16 modbusCrc16(const QByteArray &data)
{
    quint16 crc = 0xFFFF;
    for (unsigned char byte : data) {
        crc ^= byte;
        for (int i = 0; i < 8; ++i) {
            const bool lsb = (crc & 0x0001) != 0;
            crc >>= 1;
            if (lsb)
                crc ^= 0xA001;
        }
    }
    return crc;
}

bool isReadPoint(const ModbusPointDefinition &p)
{
    return p.enabled && (p.kind == ModbusPointKind::ReadOnly || p.kind == ModbusPointKind::ReadWrite);
}

void applyWordOrder(QByteArray &bytes, const QString &wordOrder)
{
    if (bytes.size() == 4 && wordOrder.compare("Swap", Qt::CaseInsensitive) == 0)
        bytes = bytes.mid(2, 2) + bytes.mid(0, 2);
}

void applyByteOrder(QByteArray &bytes, const QString &byteOrder)
{
    if (byteOrder.compare("LittleEndian", Qt::CaseInsensitive) != 0)
        return;

    if (bytes.size() == 2) {
        const char t = bytes[0];
        bytes[0] = bytes[1];
        bytes[1] = t;
    } else if (bytes.size() == 4) {
        const char t0 = bytes[0];
        bytes[0] = bytes[1];
        bytes[1] = t0;
        const char t1 = bytes[2];
        bytes[2] = bytes[3];
        bytes[3] = t1;
    }
}
}

bool decodePointValueFromResponse(const ModbusPointDefinition &point,
                                  const ModbusResponse &response,
                                  QVariant *outValue,
                                  QString *errorText)
{
    if (!outValue)
        return false;

    const QByteArray frame = response.responseFrame;
    if (frame.size() < 5) {
        if (errorText) *errorText = "response too short";
        return false;
    }

    const quint8 functionCode = static_cast<quint8>(frame.at(1));
    if (functionCode & 0x80) {
        if (errorText) *errorText = QString("modbus exception code=%1").arg(static_cast<quint8>(frame.at(2)));
        return false;
    }

    if (functionCode != static_cast<quint8>(point.functionCode)) {
        if (errorText) *errorText = QString("unexpected functionCode=%1").arg(functionCode);
        return false;
    }

    const int byteCount = static_cast<quint8>(frame.at(2));
    if (frame.size() < byteCount + 5) {
        if (errorText) *errorText = "incomplete response";
        return false;
    }

    QByteArray data = frame.mid(3, byteCount);
    const quint16 crcCalc = modbusCrc16(frame.left(3 + byteCount));
    const quint16 crcResp = static_cast<quint8>(frame.at(3 + byteCount)) |
                            (static_cast<quint8>(frame.at(4 + byteCount)) << 8);
    if (crcCalc != crcResp) {
        if (errorText) *errorText = "crc mismatch";
        return false;
    }

    const QString dt = point.dataType.toLower();
    double v = 0.0;

    if ((dt == "u16" || dt == "int16") && data.size() >= 2) {
        QByteArray raw = data.left(2);
        applyByteOrder(raw, point.byteOrder);
        const quint16 u = (static_cast<quint8>(raw[0]) << 8) | static_cast<quint8>(raw[1]);
        v = (dt == "int16") ? static_cast<qint16>(u) : static_cast<double>(u);
    } else if ((dt == "u32" || dt == "float32") && data.size() >= 4) {
        QByteArray raw = data.left(4);
        applyWordOrder(raw, point.wordOrder);
        applyByteOrder(raw, point.byteOrder);
        const quint32 u = (static_cast<quint8>(raw[0]) << 24) |
                          (static_cast<quint8>(raw[1]) << 16) |
                          (static_cast<quint8>(raw[2]) << 8) |
                          static_cast<quint8>(raw[3]);
        if (dt == "u32") {
            v = static_cast<double>(u);
        } else {
            float f = 0.0f;
            std::memcpy(&f, &u, sizeof(float));
            v = static_cast<double>(f);
        }
    } else {
        if (errorText) *errorText = QString("unsupported dataType=%1 or byteCount=%2").arg(point.dataType).arg(data.size());
        return false;
    }

    v = v * point.scale + point.offset;
    *outValue = v;
    return true;
}

SingleGroupPollRunner::SingleGroupPollRunner(ModbusController *controller,
                                             ModbusMappingConfig *config,
                                             DataBindingManager *bindingMgr,
                                             QObject *parent)
    : QObject(parent)
    , m_controller(controller)
    , m_config(config)
    , m_bindingMgr(bindingMgr)
{
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &SingleGroupPollRunner::onTick);

    if (m_controller) {
        connect(m_controller, &ModbusController::responseReady,
                this, &SingleGroupPollRunner::onResponse);
    }
}

bool SingleGroupPollRunner::start()
{
    PollGroupDefinition group;
    m_activePoints = resolveActivePoints(&group);
    if (group.id.isEmpty() || m_activePoints.isEmpty()) {
        qDebug().noquote() << "[SinglePoll] start failed: no enabled pollGroup or readable points";
        return false;
    }

    m_activeGroupId = group.id;
    m_nextIndex = 0;
    m_busy = false;
    m_pendingRequestId.clear();
    m_pendingPointByRequestId.clear();
    m_timer->start(qMax(20, group.intervalMs));

    qDebug().noquote() << QString("[SinglePoll] start group=%1 intervalMs=%2 points=%3")
                              .arg(group.id)
                              .arg(group.intervalMs)
                              .arg(m_activePoints.size());
    return true;
}

void SingleGroupPollRunner::stop()
{
    if (m_timer)
        m_timer->stop();
    m_busy = false;
    m_pendingRequestId.clear();
    m_pendingPointByRequestId.clear();
    qDebug().noquote() << QString("[SinglePoll] stop group=%1").arg(m_activeGroupId);
}

bool SingleGroupPollRunner::isRunning() const
{
    return m_timer && m_timer->isActive();
}

void SingleGroupPollRunner::onTick()
{
    qDebug().noquote() << QString("[SinglePoll] tick group=%1 busy=%2").arg(m_activeGroupId).arg(m_busy);
    if (m_busy || m_activePoints.isEmpty()) {
        if (m_busy)
            qDebug().noquote() << "[SinglePoll] skip tick because request still pending";
        return;
    }

    if (m_nextIndex >= m_activePoints.size())
        m_nextIndex = 0;

    const ModbusPointDefinition point = m_activePoints.at(m_nextIndex++);
    dispatchPointRead(point);
}

void SingleGroupPollRunner::onResponse(const ModbusResponse &response)
{
    if (!m_pendingPointByRequestId.contains(response.requestId))
        return;

    const ModbusPointDefinition point = m_pendingPointByRequestId.take(response.requestId);
    m_busy = false;
    m_pendingRequestId.clear();

    qDebug().noquote() << QString("[ModbusRead] response requestId=%1 success=%2 raw=%3")
                              .arg(response.requestId)
                              .arg(response.success)
                              .arg(QString::fromLatin1(response.responseFrame.toHex(' ')));

    if (!response.success) {
        qDebug().noquote() << QString("[ModbusRead] request failed point=%1 err=%2").arg(point.id, response.errorText);
        return;
    }

    QVariant value;
    QString err;
    if (!decodePointValueFromResponse(point, response, &value, &err)) {
        qDebug().noquote() << QString("[ModbusRead] decode failed point=%1 varId=%2 err=%3").arg(point.id, point.varId, err);
        return;
    }

    const bool updated = m_bindingMgr && m_bindingMgr->publishValue(point.varId, value);
    qDebug().noquote() << QString("[ModbusRead] value point=%1 varId=%2 parsed=%3 updateOk=%4")
                              .arg(point.id, point.varId, value.toString())
                              .arg(updated);
}

QList<ModbusPointDefinition> SingleGroupPollRunner::resolveActivePoints(PollGroupDefinition *groupOut) const
{
    QList<ModbusPointDefinition> points;
    if (!m_config)
        return points;

    PollGroupDefinition activeGroup;
    bool found = false;
    for (const auto &group : m_config->pollGroups) {
        if (!group.enabled)
            continue;
        if (!found) {
            activeGroup = group;
            found = true;
        } else {
            qDebug().noquote() << QString("[SinglePoll] multiple enabled groups, using first=%1 ignore=%2")
                                      .arg(activeGroup.id, group.id);
            break;
        }
    }

    if (!found)
        return points;

    for (const auto &point : m_config->points) {
        if (!isReadPoint(point))
            continue;
        if (point.pollGroupId != activeGroup.id)
            continue;
        points.append(point);
    }

    if (groupOut)
        *groupOut = activeGroup;

    return points;
}

void SingleGroupPollRunner::dispatchPointRead(const ModbusPointDefinition &point)
{
    if (!m_controller)
        return;

    ModbusRequest req;
    req.requestId = QString("poll_%1_%2_%3")
                        .arg(point.pollGroupId)
                        .arg(point.id)
                        .arg(QDateTime::currentMSecsSinceEpoch());
    req.type = ModbusRequestType::ReadHoldingRegisters;
    req.slaveId = point.slaveId;
    req.functionCode = point.functionCode;
    req.timeoutMs = m_config ? m_config->connection.timeoutMs : 1000;
    req.retryCount = m_config ? m_config->connection.retryCount : 0;
    req.description = QString("poll point=%1 var=%2").arg(point.id, point.varId);

    QByteArray payload;
    payload.append(static_cast<char>((point.address >> 8) & 0xFF));
    payload.append(static_cast<char>(point.address & 0xFF));
    payload.append(static_cast<char>((point.quantity >> 8) & 0xFF));
    payload.append(static_cast<char>(point.quantity & 0xFF));
    req.pduOrPayload = payload;

    m_busy = true;
    m_pendingRequestId = req.requestId;
    m_pendingPointByRequestId.insert(req.requestId, point);

    qDebug().noquote() << QString("[SinglePoll] dispatch point=%1 varId=%2 slave=%3 fc=%4 addr=%5 qty=%6 reqId=%7")
                              .arg(point.id, point.varId)
                              .arg(point.slaveId)
                              .arg(point.functionCode)
                              .arg(point.address)
                              .arg(point.quantity)
                              .arg(req.requestId);
    m_controller->enqueueRequest(req);
}
