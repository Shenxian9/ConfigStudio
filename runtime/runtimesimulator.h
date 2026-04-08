#ifndef RUNTIMESIMULATOR_H
#define RUNTIMESIMULATOR_H

#pragma once
#include <QObject>
#include <QTimer>
#include <QDateTime>
#include <QtMath>
#include <QDebug>
#include "model/variablemodel.h"

class RuntimeSimulator : public QObject
{
    Q_OBJECT
public:
    explicit RuntimeSimulator(VariableModel* model, QObject* parent = nullptr);

    void start(int intervalMs = 500);
    void stop();
    bool isRunning() const { return m_timer.isActive(); }

private slots:
    void tick();

private:
    VariableModel* m_model;
    QTimer m_timer;
};

#endif // RUNTIMESIMULATOR_H
