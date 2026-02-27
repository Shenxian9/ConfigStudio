#ifndef COMPONENTFACTORY_H
#define COMPONENTFACTORY_H
#pragma once
#include <QString>
#include "canvas/canvasitem.h"

class ComponentFactory {
public:
    static CanvasItem* create(const QString& type, QWidget *parent);
};

#endif // COMPONENTFACTORY_H
