#ifndef NUMERICCOMPONENT_H
#define NUMERICCOMPONENT_H

#pragma once
#include "canvas/canvasitem.h"
#include "canvasview.h"
#include <QLabel>
#include <QEvent>
#include <QColor>
#include <QLineEdit>

class NumericComponent : public CanvasItem {
    Q_OBJECT
public:
    explicit NumericComponent(QWidget *parent = nullptr);

    QVariantMap properties() const override;
    void setPropertyValue(const QString&, const QVariant&) override;

    QString type() const override { return "numeric"; }

    void resizeEvent(QResizeEvent *event) override;
    void changeEvent(QEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
    void updateText();   // 核心：数值 → 文本
    bool isCanvasDarkMode() const;
    void applyLabelStyle();
    void ensureInputPanel();

private:
    QLabel *m_label;

    double  m_value   = 0.0;
    int     m_decimals = 1;
    QString m_unit;
    bool m_blackBg = false;
    bool m_themeDark = false;
    QColor m_textColor = QColor("black");

    QString m_varId;

    QWidget *m_inputPanel = nullptr;
    QLineEdit *m_inputEdit = nullptr;
    bool m_userEditing = false;
};

#endif
