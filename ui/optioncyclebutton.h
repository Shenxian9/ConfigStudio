#ifndef OPTIONCYCLEBUTTON_H
#define OPTIONCYCLEBUTTON_H

#include <QPushButton>
#include <QStringList>

class OptionCycleButton : public QPushButton
{
    Q_OBJECT
public:
    explicit OptionCycleButton(QWidget *parent = nullptr);

    void addItems(const QStringList &items);
    QString currentText() const;
    int currentIndex() const { return m_currentIndex; }
    void setCurrentText(const QString &text);
    void setCurrentIndex(int index);

signals:
    void currentTextChanged(const QString &text);

private:
    QStringList m_items;
    int m_currentIndex = -1;
    void updateText();
};

#endif // OPTIONCYCLEBUTTON_H
