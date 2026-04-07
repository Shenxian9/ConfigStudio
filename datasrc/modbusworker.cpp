#include "modbusworker.h"

#include <QElapsedTimer>
#include <QMetaObject>
#include <QSerialPort>
#include <QThread>
#include <QTimer>

ModbusWorker::ModbusWorker(QObject *parent)
    : QObject(parent)
{
}

void ModbusWorker::initialize()
{
    if (m_serial)
        return;
    m_serial = new QSerialPort(this);
    m_timeoutTimer = new QTimer(this);
    m_timeoutTimer->setSingleShot(true);
    m_elapsed = new QElapsedTimer();

    connect(m_serial, &QSerialPort::readyRead, this, &ModbusWorker::onReadyRead);
    connect(m_serial, &QSerialPort::errorOccurred, this, &ModbusWorker::onSerialError);
    connect(m_timeoutTimer, &QTimer::timeout, this, &ModbusWorker::onTimeout);

    emit workerLog("[ModbusWorker] initialized");
}

void ModbusWorker::setConnectionConfig(const SerialPortConfig &cfg)
{
    m_config = cfg;
    emit workerLog(QString("[ModbusWorker] config applied port=%1 baud=%2").arg(cfg.portName).arg(cfg.baudRate));
}

void ModbusWorker::openPort()
{
    if (!m_serial)
        initialize();

    if (isMockMode()) {
        emit workerLog("[ModbusWorker] mock mode open");
        emit portStatusChanged(true);
        tryProcessNext();
        return;
    }

    if (m_serial->isOpen()) {
        emit portStatusChanged(true);
        return;
    }

    m_serial->setPortName(m_config.portName);
    m_serial->setBaudRate(m_config.baudRate);
    m_serial->setDataBits(m_config.dataBits);
    m_serial->setParity(m_config.parity);
    m_serial->setStopBits(m_config.stopBits);
    m_serial->setFlowControl(m_config.flowControl);

    const bool ok = m_serial->open(QIODevice::ReadWrite);
    emit workerLog(QString("[ModbusWorker] open port=%1 result=%2").arg(m_config.portName).arg(ok));
    emit portStatusChanged(ok);
    tryProcessNext();
}

void ModbusWorker::closePort()
{
    if (!m_serial)
        return;
    if (m_serial->isOpen())
        m_serial->close();
    emit workerLog("[ModbusWorker] close port");
    emit portStatusChanged(false);
}

void ModbusWorker::enqueueRequest(const ModbusRequest &request)
{
    m_queue.enqueue(request);
    emit workerLog(QString("[ModbusWorker] enqueue request id=%1 queue=%2")
                   .arg(request.requestId)
                   .arg(m_queue.size()));
    tryProcessNext();
}

void ModbusWorker::tryProcessNext()
{
    const bool portReady = isMockMode() || (m_serial && m_serial->isOpen());
    if (m_busy || m_queue.isEmpty() || !portReady)
        return;

    m_busy = true;
    m_current.req = m_queue.dequeue();
    m_current.retriesLeft = qMax(0, m_current.req.retryCount);
    m_current.txFrame = m_current.req.pduOrPayload;
    m_rxBuffer.clear();

    emit workerLog(QString("[ModbusWorker] start request id=%1 remainingQueue=%2")
                   .arg(m_current.req.requestId)
                   .arg(m_queue.size()));
    startCurrentRequest();
}

void ModbusWorker::startCurrentRequest()
{
    if (m_elapsed)
        m_elapsed->restart();

    if (isMockMode()) {
        emit workerLog(QString("[ModbusWorker] mock send id=%1 frame=%2")
                       .arg(m_current.req.requestId, QString::fromLatin1(m_current.txFrame.toHex(' '))));
        QTimer::singleShot(30, this, [this]() {
            if (!m_busy)
                return;
            m_rxBuffer = QByteArray("MOCK:") + m_current.txFrame;
            completeCurrent(true);
        });
        return;
    }

    if (!m_serial || !m_serial->isOpen()) {
        completeCurrent(false, QStringLiteral("serial port not open"));
        return;
    }

    const qint64 written = m_serial->write(m_current.txFrame);
    if (written < 0) {
        completeCurrent(false, m_serial->errorString());
        return;
    }
    emit workerLog(QString("[ModbusWorker] send id=%1 bytes=%2 frame=%3")
                   .arg(m_current.req.requestId)
                   .arg(written)
                   .arg(QString::fromLatin1(m_current.txFrame.toHex(' '))));

    m_timeoutTimer->start(qMax(10, m_current.req.timeoutMs));
}

void ModbusWorker::onReadyRead()
{
    if (!m_busy || !m_serial)
        return;

    m_rxBuffer.append(m_serial->readAll());
    emit workerLog(QString("[ModbusWorker] recv id=%1 bytes=%2")
                   .arg(m_current.req.requestId)
                   .arg(m_rxBuffer.size()));

    if (m_config.frameTerminator.isEmpty()) {
        completeCurrent(true);
        return;
    }

    if (m_rxBuffer.contains(m_config.frameTerminator))
        completeCurrent(true);
}

void ModbusWorker::onTimeout()
{
    if (!m_busy)
        return;

    emit workerLog(QString("[ModbusWorker] timeout id=%1 retriesLeft=%2")
                   .arg(m_current.req.requestId)
                   .arg(m_current.retriesLeft));

    if (m_current.retriesLeft > 0) {
        --m_current.retriesLeft;
        startCurrentRequest();
        return;
    }

    completeCurrent(false, QStringLiteral("request timeout"));
}

void ModbusWorker::onSerialError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::NoError)
        return;

    emit workerLog(QString("[ModbusWorker] serial error=%1").arg(static_cast<int>(error)));
    if (m_busy && m_serial)
        completeCurrent(false, m_serial->errorString());
}

void ModbusWorker::completeCurrent(bool success, const QString &errorText)
{
    if (!m_busy)
        return;

    if (m_timeoutTimer)
        m_timeoutTimer->stop();

    ModbusResponse resp;
    resp.requestId = m_current.req.requestId;
    resp.success = success;
    resp.requestFrame = m_current.txFrame;
    resp.responseFrame = m_rxBuffer;
    resp.errorText = errorText;
    resp.elapsedMs = m_elapsed ? static_cast<int>(m_elapsed->elapsed()) : 0;

    emit workerLog(QString("[ModbusWorker] complete id=%1 success=%2 elapsedMs=%3 queue=%4")
                   .arg(resp.requestId)
                   .arg(resp.success)
                   .arg(resp.elapsedMs)
                   .arg(m_queue.size()));
    emit responseReady(resp);

    m_busy = false;
    m_current = PendingRequest();
    m_rxBuffer.clear();
    tryProcessNext();
}

bool ModbusWorker::isMockMode() const
{
    return m_config.portName.compare("mock", Qt::CaseInsensitive) == 0;
}

ModbusController::ModbusController(QObject *parent)
    : QObject(parent)
{
    qRegisterMetaType<SerialPortConfig>("SerialPortConfig");
    qRegisterMetaType<ModbusRequest>("ModbusRequest");
    qRegisterMetaType<ModbusResponse>("ModbusResponse");

    m_thread = new QThread(this);
    m_worker = new ModbusWorker();
    m_worker->moveToThread(m_thread);

    connect(m_thread, &QThread::started, m_worker, &ModbusWorker::initialize);
    connect(m_thread, &QThread::finished, m_worker, &QObject::deleteLater);

    connect(this, &ModbusController::requestSetConfig, m_worker, &ModbusWorker::setConnectionConfig, Qt::QueuedConnection);
    connect(this, &ModbusController::requestOpenPort, m_worker, &ModbusWorker::openPort, Qt::QueuedConnection);
    connect(this, &ModbusController::requestClosePort, m_worker, &ModbusWorker::closePort, Qt::QueuedConnection);
    connect(this, &ModbusController::requestEnqueue, m_worker, &ModbusWorker::enqueueRequest, Qt::QueuedConnection);

    connect(m_worker, &ModbusWorker::responseReady, this, &ModbusController::responseReady, Qt::QueuedConnection);
    connect(m_worker, &ModbusWorker::portStatusChanged, this, &ModbusController::portStatusChanged, Qt::QueuedConnection);
    connect(m_worker, &ModbusWorker::workerLog, this, &ModbusController::workerLog, Qt::QueuedConnection);

    m_thread->start();
}

ModbusController::~ModbusController()
{
    if (m_thread && m_thread->isRunning()) {
        emit requestClosePort();
        m_thread->quit();
        m_thread->wait(2000);
    }
}

void ModbusController::applyConnectionConfig(const SerialPortConfig &cfg)
{
    emit requestSetConfig(cfg);
}

void ModbusController::openPort()
{
    emit requestOpenPort();
}

void ModbusController::closePort()
{
    emit requestClosePort();
}

void ModbusController::enqueueRequest(const ModbusRequest &request)
{
    emit requestEnqueue(request);
}
