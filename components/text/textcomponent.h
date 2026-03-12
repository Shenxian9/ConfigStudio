#ifndef TEXTCOMPONENT_H
#define TEXTCOMPONENT_H

#pragma once
#include "canvas/canvasitem.h"
#include <QLabel>
#include <QEvent>

class TextComponent : public CanvasItem {
    Q_OBJECT
public:
    explicit TextComponent(QWidget *parent = nullptr);

    QVariantMap properties() const override;
    void setPropertyValue(const QString&, const QVariant&) override;

    QString type() const override { return "text"; }

    void resizeEvent(QResizeEvent *event) override;
    void changeEvent(QEvent *event) override;

private:
    bool isCanvasDarkMode() const;
    void applyLabelStyle();

private:
    QLabel *m_label;
    bool m_blackBg = false;
    bool m_themeDark = false;
};

#endif
