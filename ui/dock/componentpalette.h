#ifndef COMPONENTPALETTE_H
#define COMPONENTPALETTE_H

#pragma once
#include <QLabel>

class ComponentPalette : public QLabel {
    Q_OBJECT
public:
    explicit ComponentPalette(const QString& type, QWidget *parent = nullptr);

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    QString m_type;
};


#endif // COMPONENTPALETTE_H
