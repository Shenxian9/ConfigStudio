#include "databindingmanager.h"
#include <QDebug>
#include <algorithm>

DataBindingManager::DataBindingManager(VariableModel* model, QObject* parent)
    : QObject(parent), m_model(model)
{
    Q_ASSERT(model);

    bool ok = connect(model, &VariableModel::variableValueChanged,
                      this, &DataBindingManager::onVariableChanged);


    qDebug() << "connect result =" << ok;
    qDebug() << "BindingMgr model =" << m_model;
}

void DataBindingManager::bind(const QString& varId, CanvasItem* item, const QString& property)
{
    if (!item)
        return;

    auto& list = m_bindings[varId];
    const auto alreadyBound = std::any_of(list.cbegin(), list.cend(),
                                          [&](const Binding& b) {
                                              return b.item == item && b.property == property;
                                          });
    if (alreadyBound)
        return;

    Binding b;
    b.item = item;
    b.property = property;

    list.append(b);

    // ⭐ 控件销毁时自动移除
    connect(item, &QObject::destroyed, this, [this, varId, item]() {
        auto it = m_bindings.find(varId);
        if (it == m_bindings.end())
            return;

        auto& itemBindings = it.value();
        itemBindings.erase(std::remove_if(itemBindings.begin(), itemBindings.end(),
                                          [item](const Binding& b) { return b.item == item; }),
                           itemBindings.end());

        if (itemBindings.isEmpty())
            m_bindings.erase(it);
    });
}

void DataBindingManager::unbind(const QString& varId, CanvasItem* item, const QString& property)
{
    if (!m_bindings.contains(varId))
        return;

    auto &list = m_bindings[varId];
    list.erase(std::remove_if(list.begin(), list.end(),
                              [&](const Binding& b){ return b.item == item && b.property == property; }),
               list.end());

    if (list.isEmpty())
        m_bindings.remove(varId);
}


void DataBindingManager::onVariableChanged(const QString& varId, const QVariant& value)
{
    auto it = m_bindings.find(varId);
    if (it == m_bindings.end())
        return;

    auto& list = it.value();

    for (int i = list.size() - 1; i >= 0; --i) {
        Binding& b = list[i];

        // ⭐ 控件已被删除 → 自动为 nullptr
        if (!b.item) {
            list.removeAt(i);   // 清理失效绑定
            continue;
        }

        b.item->setPropertyValue(b.property, value);
    }

    if (list.isEmpty())
        m_bindings.erase(it);
}

