#include "optioncyclebutton.h"

OptionCycleButton::OptionCycleButton(QWidget *parent)
    : QPushButton(parent)
{
    connect(this, &QPushButton::clicked, this, [this]() {
        if (m_items.isEmpty())
            return;
        setCurrentIndex((m_currentIndex + 1) % m_items.size());
    });
}

void OptionCycleButton::addItems(const QStringList &items)
{
    m_items = items;
    if (m_items.isEmpty()) {
        m_currentIndex = -1;
        setText(QString());
        return;
    }
    if (m_currentIndex < 0 || m_currentIndex >= m_items.size())
        m_currentIndex = 0;
    updateText();
}

QString OptionCycleButton::currentText() const
{
    if (m_currentIndex < 0 || m_currentIndex >= m_items.size())
        return QString();
    return m_items.at(m_currentIndex);
}

void OptionCycleButton::setCurrentText(const QString &text)
{
    const int idx = m_items.indexOf(text);
    if (idx >= 0) {
        setCurrentIndex(idx);
    }
}

void OptionCycleButton::setCurrentIndex(int index)
{
    if (m_items.isEmpty())
        return;
    if (index < 0 || index >= m_items.size())
        return;
    if (m_currentIndex == index)
        return;
    m_currentIndex = index;
    updateText();
    emit currentTextChanged(currentText());
}

void OptionCycleButton::updateText()
{
    setText(currentText());
}
