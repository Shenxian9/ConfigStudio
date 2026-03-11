#ifndef TEXTCOMPONENT_H
#define TEXTCOMPONENT_H

#pragma once
#include "canvas/canvasitem.h"
#include <QLabel>

class TextComponent : public CanvasItem {
    Q_OBJECT
public:
    explicit TextComponent(QWidget *parent = nullptr);

    QVariantMap properties() const override;
    void setPropertyValue(const QString&, const QVariant&) override;

    QString type() const override { return "text"; }

    void resizeEvent(QResizeEvent *event) override;

private:
    QLabel *m_label;
    bool m_blackBg = false;
};

#endif
