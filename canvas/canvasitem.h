#ifndef CANVASITEM_H
#define CANVASITEM_H

#include <QWidget>
#include <QMouseEvent>
#include <QDebug>
#include <QPoint>
#include <QStyle>
#include <QStyleOption>
#include <QChildEvent>
#include <QFrame>
#include <QString>
class DataBindingManager;

class CanvasItem : public QWidget {
    Q_OBJECT
public:
    explicit CanvasItem(QWidget *parent = nullptr);

    virtual QString type() const = 0;
    virtual QVariantMap properties() const = 0;
    virtual void setPropertyValue(const QString& key, const QVariant& value) = 0;

    void setSelected(bool sel);
    bool isSelected() const { return m_selected; }

    void setBindingManager(DataBindingManager* mgr) { m_bindingMgr = mgr; }
    void setEditLocked(bool locked);
    bool isEditLocked() const { return m_editLocked; }
    QString itemId() const { return m_itemId; }
    void setItemId(const QString &id);


signals:
    void selected(CanvasItem *self);

protected:
    QPoint m_dragStart;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void childEvent(QChildEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;


    DataBindingManager* m_bindingMgr = nullptr;
private:
    QString m_itemId;
    bool m_selected = false;

    // Resize handle
    const int handleSize = 30;
    bool m_resizing = false;
    bool m_dragging = false;
    bool m_editLocked = false;

    QRect m_startRect;

    bool isInResizeHandle(const QPoint& pos) const;
    void updateSelectionOverlay();

    QFrame *m_selectionFrame = nullptr;
    QWidget *m_resizeHandleOverlay = nullptr;

};

#endif // CANVASITEM_H
