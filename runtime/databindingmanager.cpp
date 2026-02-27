#include "databindingmanager.h"

DataBindingManager::DataBindingManager(VariableModel* model, QObject* parent)
    : QObject(parent), m_model(model)
{
    bool ok = connect(model, &VariableModel::variableValueChanged,
                      this, &DataBindingManager::onVariableChanged);


    qDebug() << "connect result =" << ok;
    qDebug() << "BindingMgr model =" << m_model;
}

void DataBindingManager::bind(const QString& varId, CanvasItem* item, const QString& property)
{
    Binding b;
    b.item = item;
    b.property = property;

    m_bindings[varId].append(b);

    // ⭐ 控件销毁时自动移除
    connect(item, &QObject::destroyed, this, [this, varId, item]() {
        auto& list = m_bindings[varId];
        list.erase(std::remove_if(list.begin(), list.end(),
                                  [item](const Binding& b){ return b.item == item; }),
                   list.end());
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
    if (!m_bindings.contains(varId))
        return;

    auto& list = m_bindings[varId];

    for (int i = list.size() - 1; i >= 0; --i) {
        Binding& b = list[i];

        // ⭐ 控件已被删除 → 自动为 nullptr
        if (!b.item) {
            list.removeAt(i);   // 清理失效绑定
            continue;
        }

        b.item->setPropertyValue(b.property, value);
    }
}


