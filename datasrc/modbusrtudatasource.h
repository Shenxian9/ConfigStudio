#ifndef MODBUSRTUDATASOURCE_H
#define MODBUSRTUDATASOURCE_H

#include <QObject>

// Phase 1 placeholder: this class will replace the temporary SerialDataSource
// (text frame + key/value parsing) in a later phase with real Modbus RTU logic.
class ModbusRtuDataSource : public QObject
{
    Q_OBJECT
public:
    explicit ModbusRtuDataSource(QObject *parent = nullptr);
};

#endif // MODBUSRTUDATASOURCE_H
