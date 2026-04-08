#ifndef SINGLEGROUPPOLLRUNNER_H
#define SINGLEGROUPPOLLRUNNER_H

#pragma once

#include <QObject>
#include <QHash>

#include "modbusmappingdefs.h"
#include "modbusworker.h"

class QTimer;
class DataBindingManager;

bool decodePointValueFromResponse(const ModbusPointDefinition &point,
                                  const ModbusResponse &response,
                                  QVariant *outValue,
                                  QString *errorText);

class SingleGroupPollRunner : public QObject
{
    Q_OBJECT
public:
    explicit SingleGroupPollRunner(ModbusController *controller,
                                   ModbusMappingConfig *config,
                                   DataBindingManager *bindingMgr,
                                   QObject *parent = nullptr);

    bool start();
    void stop();
    bool isRunning() const;

private slots:
    void onTick();
    void onResponse(const ModbusResponse &response);

private:
    QList<ModbusPointDefinition> resolveActivePoints(PollGroupDefinition *groupOut = nullptr) const;
    void dispatchPointRead(const ModbusPointDefinition &point);

    ModbusController *m_controller = nullptr;
    ModbusMappingConfig *m_config = nullptr;
    DataBindingManager *m_bindingMgr = nullptr;
    QTimer *m_timer = nullptr;

    QString m_activeGroupId;
    QList<ModbusPointDefinition> m_activePoints;
    int m_nextIndex = 0;
    bool m_busy = false;
    QString m_pendingRequestId;
    QHash<QString, ModbusPointDefinition> m_pendingPointByRequestId;
};

#endif // SINGLEGROUPPOLLRUNNER_H
