#ifndef WHEELCOMPONENT_H
#define WHEELCOMPONENT_H

#pragma once
#include "canvas/canvasitem.h"
#include <qwt_wheel.h>
#include <QLabel>

class WheelComponent : public CanvasItem {
    Q_OBJECT
public:
    explicit WheelComponent(QWidget *parent = nullptr);

    QVariantMap properties() const override;
    void setPropertyValue(const QString&, const QVariant&) override;

    QString type() const override { return "wheel"; }

    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    QLabel   *m_title;
    QwtWheel *m_wheel;
    QLabel   *m_value;

    QString m_varId;
    bool m_updatingFromBinding = false;
    bool m_userInteracting = false;
};

#endif
