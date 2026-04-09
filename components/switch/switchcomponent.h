#ifndef SWITCHCOMPONENT_H
#define SWITCHCOMPONENT_H

#pragma once
#include "canvas/canvasitem.h"
#include <QLabel>
#include <QWidget>
class QMouseEvent;

class SwitchVisual : public QWidget
{
    Q_OBJECT
public:
    explicit SwitchVisual(QWidget *parent = nullptr);
    bool isChecked() const { return m_checked; }
    void setChecked(bool checked);

signals:
    void toggled(bool checked);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    bool m_checked = false;
};

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
    SwitchVisual *m_switch;
};

#endif
