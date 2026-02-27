#ifndef SWITCHCOMPONENT_H
#define SWITCHCOMPONENT_H

#pragma once
#include "canvas/canvasitem.h"
#include <QCheckBox>
#include <QLabel>

class SwitchComponent : public CanvasItem {
    Q_OBJECT
public:
    explicit SwitchComponent(QWidget *parent = nullptr);

    QVariantMap properties() const override;
    void setPropertyValue(const QString&, const QVariant&) override;

    QString type() const override { return "switch"; }

    void resizeEvent(QResizeEvent *event) override;

private:
    QLabel     *m_title;
    QCheckBox  *m_switch;
};

#endif
