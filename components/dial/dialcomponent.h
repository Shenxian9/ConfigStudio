#ifndef DIALCOMPONENT_H
#define DIALCOMPONENT_H

#pragma once
#include "canvas/canvasitem.h"
#include "canvasview.h"
#include <qwt_dial.h>
#include <qwt_scale_draw.h>
#include <qwt_round_scale_draw.h>
#include <QLabel>



class DialComponent : public CanvasItem {
    Q_OBJECT
public:
    explicit DialComponent(QWidget *parent = nullptr);

    QVariantMap properties() const override;
    void setPropertyValue(const QString&, const QVariant&) override;

    QString type() const override { return "dial"; }

    void resizeEvent(QResizeEvent* event) override;
    void changeEvent(QEvent* event) override;

private:
    void applyDialPalette();

    QwtDial *m_dial;
    QLabel  *m_title;
    QLabel  *m_valueLabel;

    QString m_varId;
};

#endif
