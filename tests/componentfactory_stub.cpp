#include "components/base/ComponentFactory.h"

namespace {
class DummyCanvasItem : public CanvasItem
{
public:
    explicit DummyCanvasItem(QWidget *parent = nullptr) : CanvasItem(parent) {}
    QString type() const override { return "dummy"; }
    QVariantMap properties() const override { return {}; }
    void setPropertyValue(const QString &, const QVariant &) override {}
};
}

CanvasItem *ComponentFactory::create(const QString &, QWidget *parent)
{
    return new DummyCanvasItem(parent);
}
