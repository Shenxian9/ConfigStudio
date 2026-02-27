#ifndef PALETTEBINDER_H
#define PALETTEBINDER_H

#pragma once
#include <QObject>
#include <QLabel>
#include <QMap>

class PaletteBinder : public QObject {
    Q_OBJECT
public:
    explicit PaletteBinder(QObject *parent = nullptr);

    void bind(QLabel *label, const QString &type);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    QMap<QLabel*, QString> m_map;
};

#endif // PALETTEBINDER_H
