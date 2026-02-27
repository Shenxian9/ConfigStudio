#ifndef VARIABLE_H
#define VARIABLE_H
// model/Variable.h
#pragma once
#include <QString>
#include <QVariant>

enum class RegisterArea {
    Coil,
    DiscreteInput,
    InputRegister,
    HoldingRegister
};

enum class DataStrategy {
    None,
    Fixed,
    Sine,
    Square,
    Random
};

struct Variable {
    // 基础属性
    QString id;          // 全局唯一ID
    QString name;        // 变量名
    QString deviceId;    // 所属设备
    QString type;        // int/float/bool
    QString unit;

    // 运行态
    QVariant value;
    qint64 timestamp = 0;

    // 通讯映射
    RegisterArea area;
    int address = 0;
    int count = 1;
    int bitOffset = 0;
    int bitWidth = 16;

    // 工程逻辑
    double lowLimit = 0;
    double highLimit = 0;

    // 仿真
    DataStrategy strategy = DataStrategy::None;
};
#endif // VARIABLE_H
