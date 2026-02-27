#include "runtimesimulator.h"
#include <QtMath>
#include <QRandomGenerator>

RuntimeSimulator::RuntimeSimulator(VariableModel* model, QObject* parent)
    : QObject(parent), m_model(model)
{
    connect(&m_timer, &QTimer::timeout, this, &RuntimeSimulator::tick);
    qDebug() << "Simulator model =" << m_model;
}

void RuntimeSimulator::start(int intervalMs)
{
    m_timer.start(intervalMs);
}

void RuntimeSimulator::tick()
{
    for (int i = 0; i < m_model->rowCount({}); ++i) {
        Variable& var = m_model->variableAt(i);


        double newValue = 0;


        switch (var.strategy) {
        case DataStrategy::Fixed:
            newValue = 50;
            break;


        case DataStrategy::Sine:
            newValue = 50 + 50 * qSin(QDateTime::currentMSecsSinceEpoch() / 500.0);
            break;


        case DataStrategy::Square:
            newValue = (QDateTime::currentMSecsSinceEpoch() / 1000 % 2) ? 100 : 0;
            break;


        case DataStrategy::Random:
            newValue = QRandomGenerator::global()->generateDouble() * 100.0;
            break;


        default:
            continue;
        }


        // ⭐ 加在这里
        //qDebug() << "Simulator set:" << var.id << newValue;


        m_model->updateValue(i, newValue);
    }
}
