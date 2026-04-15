#include "projectfilemanager.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QRegularExpression>
#include <QSet>
#include <algorithm>

namespace {
QString registerAreaToString(RegisterArea area)
{
    switch (area) {
    case RegisterArea::Coil: return "Coil";
    case RegisterArea::DiscreteInput: return "DiscreteInput";
    case RegisterArea::InputRegister: return "InputRegister";
    case RegisterArea::HoldingRegister:
    default:
        return "HoldingRegister";
    }
}

RegisterArea registerAreaFromString(const QString &area)
{
    const QString normalized = area.trimmed();
    if (normalized.compare("Coil", Qt::CaseInsensitive) == 0) return RegisterArea::Coil;
    if (normalized.compare("DiscreteInput", Qt::CaseInsensitive) == 0) return RegisterArea::DiscreteInput;
    if (normalized.compare("InputRegister", Qt::CaseInsensitive) == 0) return RegisterArea::InputRegister;
    return RegisterArea::HoldingRegister;
}

QString endiannessToString(Endianness e)
{
    switch (e) {
    case Endianness::BigEndianWordSwap: return "BigEndianWordSwap";
    case Endianness::LittleEndian: return "LittleEndian";
    case Endianness::LittleEndianByteSwap: return "LittleEndianByteSwap";
    case Endianness::BigEndian:
    default:
        return "BigEndian";
    }
}

Endianness endiannessFromString(const QString &v)
{
    if (v.trimmed().compare("BigEndianWordSwap", Qt::CaseInsensitive) == 0)
        return Endianness::BigEndianWordSwap;
    if (v.trimmed().compare("LittleEndian", Qt::CaseInsensitive) == 0)
        return Endianness::LittleEndian;
    if (v.trimmed().compare("LittleEndianByteSwap", Qt::CaseInsensitive) == 0)
        return Endianness::LittleEndianByteSwap;
    return Endianness::BigEndian;
}
}

bool ProjectFileManager::saveProject(const QString &filePath,
                                    CanvasView *canvas,
                                    VariableModel *variableModel,
                                    DataBindingManager *bindingMgr,
                                    const QVector<SerialPortConfig> &modbusConfigs,
                                    const SerialPortConfig &activeConfig,
                                    QString *errorText) const
{
    if (!canvas || !variableModel || !bindingMgr) {
        if (errorText) *errorText = QObject::tr("invalid save context");
        return false;
    }

    QJsonObject root;
    root["version"] = kProjectVersion;

    QJsonObject canvasObj;
    canvasObj["width"] = canvas->width();
    canvasObj["height"] = canvas->height();

    QJsonArray itemsArray;
    QHash<QString, QJsonArray> bindingByItem;
    const QList<DataBindingManager::BindingInfo> runtimeBindings = bindingMgr->bindingInfos();
    for (const DataBindingManager::BindingInfo &info : runtimeBindings) {
        if (info.itemId.isEmpty() || info.varId.trimmed().isEmpty() || info.property.trimmed().isEmpty())
            continue;
        QJsonObject b;
        b["property"] = info.property.trimmed();
        b["varId"] = info.varId.trimmed();
        bindingByItem[info.itemId].append(b);
    }
    const auto items = canvas->canvasItems();
    for (int i = 0; i < items.size(); ++i) {
        CanvasItem *item = items.at(i);
        if (!item)
            continue;
        const QString itemId = ensureItemId(item, i + 1);
        QJsonObject itemObj = serializeItem(item, i);
        if (bindingByItem.contains(itemId)) {
            QJsonArray merged = itemObj.value("bindings").toArray();
            QSet<QString> seen;
            for (const QJsonValue &v : merged) {
                const QJsonObject o = v.toObject();
                seen.insert(o.value("property").toString() + "@@" + o.value("varId").toString());
            }
            for (const QJsonValue &v : bindingByItem.value(itemId)) {
                const QJsonObject o = v.toObject();
                const QString key = o.value("property").toString() + "@@" + o.value("varId").toString();
                if (seen.contains(key))
                    continue;
                seen.insert(key);
                merged.append(o);
            }
            itemObj["bindings"] = merged;
        }
        itemsArray.append(itemObj);
    }
    canvasObj["items"] = itemsArray;
    root["canvas"] = canvasObj;

    QJsonArray varsArray;
    const auto vars = variableModel->variables();
    for (const Variable &var : vars)
        varsArray.append(serializeVariable(var));
    root["variables"] = varsArray;

    QJsonObject modbusObj;
    modbusObj["activeConfig"] = serializeModbusConfig(activeConfig);
    QJsonArray cfgs;
    for (const SerialPortConfig &cfg : modbusConfigs)
        cfgs.append(serializeModbusConfig(cfg));
    modbusObj["configs"] = cfgs;
    root["modbus"] = modbusObj;

    QFile f(filePath);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (errorText) *errorText = QObject::tr("cannot open file for write: %1").arg(f.errorString());
        return false;
    }

    const QJsonDocument doc(root);
    const qint64 written = f.write(doc.toJson(QJsonDocument::Indented));
    f.close();
    if (written <= 0) {
        if (errorText) *errorText = QObject::tr("write project file failed");
        return false;
    }
    return true;
}

bool ProjectFileManager::loadProject(const QString &filePath,
                                    CanvasView *canvas,
                                    VariableModel *variableModel,
                                    DataBindingManager *bindingMgr,
                                    QVector<SerialPortConfig> *modbusConfigs,
                                    SerialPortConfig *activeConfig,
                                    QStringList *warnings,
                                    QString *errorText) const
{
    if (!canvas || !variableModel || !bindingMgr || !modbusConfigs || !activeConfig) {
        if (errorText) *errorText = QObject::tr("invalid load context");
        return false;
    }

    QFile f(filePath);
    if (!f.open(QIODevice::ReadOnly)) {
        if (errorText) *errorText = QObject::tr("cannot open file: %1").arg(f.errorString());
        return false;
    }
    const QByteArray bytes = f.readAll();
    f.close();

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(bytes, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        if (errorText) *errorText = QObject::tr("json parse failed: %1").arg(parseError.errorString());
        return false;
    }

    const QJsonObject root = doc.object();
    const int version = root.value("version").toInt(kProjectVersion);
    if (version > kProjectVersion && warnings)
        warnings->append(QObject::tr("project version %1 is newer than supported %2").arg(version).arg(kProjectVersion));

    variableModel->clear();
    const QJsonArray varsArray = root.value("variables").toArray();
    for (const QJsonValue &v : varsArray) {
        if (!v.isObject())
            continue;
        Variable var;
        if (deserializeVariable(v.toObject(), &var)) {
            variableModel->addVariable(var);
        } else if (warnings) {
            warnings->append(QObject::tr("skip invalid variable entry"));
        }
    }

    modbusConfigs->clear();
    const QJsonObject modbusObj = root.value("modbus").toObject();
    const QJsonArray cfgs = modbusObj.value("configs").toArray();
    for (const QJsonValue &v : cfgs) {
        if (v.isObject())
            modbusConfigs->append(deserializeModbusConfig(v.toObject()));
    }
    if (modbusConfigs->isEmpty())
        modbusConfigs->append(deserializeModbusConfig(modbusObj.value("activeConfig").toObject()));
    *activeConfig = deserializeModbusConfig(modbusObj.value("activeConfig").toObject());
    if (activeConfig->portName.trimmed().isEmpty() && !modbusConfigs->isEmpty())
        *activeConfig = modbusConfigs->first();

    canvas->clearAllItems();

    const QJsonObject canvasObj = root.value("canvas").toObject();
    const int canvasW = canvasObj.value("width").toInt(canvas->width());
    const int canvasH = canvasObj.value("height").toInt(canvas->height());
    if (canvasW > 0 && canvasH > 0)
        canvas->resize(canvasW, canvasH);
    const QJsonArray itemsArray = canvasObj.value("items").toArray();
    QVector<QJsonObject> itemObjs;
    itemObjs.reserve(itemsArray.size());
    for (const QJsonValue &itemValue : itemsArray) {
        if (itemValue.isObject())
            itemObjs.append(itemValue.toObject());
    }
    std::sort(itemObjs.begin(), itemObjs.end(), [](const QJsonObject &a, const QJsonObject &b) {
        return a.value("z").toInt() < b.value("z").toInt();
    });

    for (const QJsonObject &itemObj : itemObjs) {
        const QString type = itemObj.value("type").toString();
        const QString itemId = itemObj.value("id").toString();
        if (type.isEmpty()) {
            if (warnings) warnings->append(QObject::tr("skip item with empty type"));
            continue;
        }

        const QPoint pos(itemObj.value("x").toInt(), itemObj.value("y").toInt());
        CanvasItem *item = canvas->createItem(type, pos);
        if (!item) {
            if (warnings) warnings->append(QObject::tr("unknown component type: %1").arg(type));
            continue;
        }
        item->setObjectName(itemId.isEmpty() ? ensureItemId(item, 0) : itemId);

        const int w = qMax(20, itemObj.value("width").toInt(item->width()));
        const int h = qMax(20, itemObj.value("height").toInt(item->height()));
        item->resize(w, h);
        item->setVisible(itemObj.value("visible").toBool(true));

        applyItemProperties(item, itemObj, warnings);
        applyBindings(item, itemObj.value("bindings").toArray(), bindingMgr, warnings);
    }

    canvas->clearSelection();
    return true;
}

QJsonObject ProjectFileManager::serializeVariable(const Variable &var) const
{
    QJsonObject obj;
    obj["id"] = var.id;
    obj["name"] = var.name;
    obj["deviceId"] = var.deviceId;
    obj["type"] = var.type;
    obj["unit"] = var.unit;
    obj["value"] = QJsonValue::fromVariant(var.value);
    obj["timestamp"] = QString::number(var.timestamp);
    obj["area"] = registerAreaToString(var.area);
    obj["address"] = var.address;
    obj["count"] = var.count;
    obj["bitOffset"] = var.bitOffset;
    obj["bitWidth"] = var.bitWidth;
    obj["scale"] = var.scale;
    obj["readOnly"] = var.readOnly;
    obj["endianness"] = endiannessToString(var.endianness);
    obj["lowLimit"] = var.lowLimit;
    obj["highLimit"] = var.highLimit;
    obj["strategy"] = static_cast<int>(var.strategy);
    return obj;
}

bool ProjectFileManager::deserializeVariable(const QJsonObject &obj, Variable *var) const
{
    if (!var)
        return false;

    const QString id = obj.value("id").toString().trimmed();
    if (id.isEmpty())
        return false;

    var->id = id;
    var->name = obj.value("name").toString();
    var->deviceId = obj.value("deviceId").toString();
    var->type = obj.value("type").toString("float32");
    var->unit = obj.value("unit").toString();
    var->value = obj.value("value").toVariant();
    var->timestamp = obj.value("timestamp").toString().toLongLong();
    var->area = registerAreaFromString(obj.value("area").toString());
    var->address = obj.value("address").toInt();
    var->count = qMax(1, obj.value("count").toInt(1));
    var->bitOffset = qMax(0, obj.value("bitOffset").toInt(0));
    var->bitWidth = qMax(1, obj.value("bitWidth").toInt(16));
    var->scale = obj.value("scale").toDouble(1.0);
    if (var->scale <= 0.0)
        var->scale = 1.0;
    var->readOnly = obj.value("readOnly").toBool(true);
    var->endianness = endiannessFromString(obj.value("endianness").toString());
    var->lowLimit = obj.value("lowLimit").toDouble();
    var->highLimit = obj.value("highLimit").toDouble();
    var->strategy = static_cast<DataStrategy>(obj.value("strategy").toInt(0));
    return true;
}

QJsonObject ProjectFileManager::serializeModbusConfig(const SerialPortConfig &cfg) const
{
    QJsonObject obj;
    obj["deviceId"] = cfg.deviceId;
    obj["portName"] = cfg.portName;
    obj["baudRate"] = cfg.baudRate;
    obj["dataBits"] = static_cast<int>(cfg.dataBits);
    obj["parity"] = static_cast<int>(cfg.parity);
    obj["stopBits"] = static_cast<int>(cfg.stopBits);
    obj["flowControl"] = static_cast<int>(cfg.flowControl);
    obj["slaveId"] = cfg.slaveId;
    obj["timeoutMs"] = cfg.timeoutMs;
    obj["retryCount"] = cfg.retryCount;
    obj["pollIntervalMs"] = cfg.pollIntervalMs;
    obj["defaultFunctionCode"] = cfg.defaultFunctionCode;
    obj["frameTerminator"] = QString::fromUtf8(cfg.frameTerminator.toBase64());
    return obj;
}

SerialPortConfig ProjectFileManager::deserializeModbusConfig(const QJsonObject &obj) const
{
    SerialPortConfig cfg;
    if (obj.isEmpty())
        return cfg;

    cfg.deviceId = obj.value("deviceId").toString(cfg.deviceId);
    cfg.portName = obj.value("portName").toString(cfg.portName);
    cfg.baudRate = obj.value("baudRate").toInt(cfg.baudRate);
    cfg.dataBits = static_cast<QSerialPort::DataBits>(obj.value("dataBits").toInt(static_cast<int>(cfg.dataBits)));
    cfg.parity = static_cast<QSerialPort::Parity>(obj.value("parity").toInt(static_cast<int>(cfg.parity)));
    cfg.stopBits = static_cast<QSerialPort::StopBits>(obj.value("stopBits").toInt(static_cast<int>(cfg.stopBits)));
    cfg.flowControl = static_cast<QSerialPort::FlowControl>(obj.value("flowControl").toInt(static_cast<int>(cfg.flowControl)));
    cfg.slaveId = obj.value("slaveId").toInt(cfg.slaveId);
    cfg.timeoutMs = obj.value("timeoutMs").toInt(cfg.timeoutMs);
    cfg.retryCount = obj.value("retryCount").toInt(cfg.retryCount);
    cfg.pollIntervalMs = obj.value("pollIntervalMs").toInt(cfg.pollIntervalMs);
    cfg.defaultFunctionCode = obj.value("defaultFunctionCode").toInt(cfg.defaultFunctionCode);
    const QByteArray frameTerm = QByteArray::fromBase64(obj.value("frameTerminator").toString().toUtf8());
    if (!frameTerm.isEmpty())
        cfg.frameTerminator = frameTerm;
    return cfg;
}

QJsonObject ProjectFileManager::serializeItem(CanvasItem *item, int zIndex) const
{
    QJsonObject obj;
    obj["id"] = ensureItemId(item, zIndex + 1);
    obj["type"] = item->type();
    obj["x"] = item->x();
    obj["y"] = item->y();
    obj["width"] = item->width();
    obj["height"] = item->height();
    obj["z"] = zIndex;
    obj["visible"] = item->isVisible();

    const QVariantMap props = item->properties();
    obj["properties"] = QJsonObject::fromVariantMap(props);

    QJsonArray bindings;
    static QRegularExpression varIdRe("^varId(\\d+)?$");
    for (auto it = props.cbegin(); it != props.cend(); ++it) {
        const QRegularExpressionMatch m = varIdRe.match(it.key());
        if (!m.hasMatch())
            continue;
        const QString varId = it.value().toString().trimmed();
        if (varId.isEmpty())
            continue;

        QString idx = m.captured(1);
        QString bindingProp = "value";
        if (!idx.isEmpty())
            bindingProp = QString("value%1").arg(idx);

        QJsonObject b;
        b["property"] = bindingProp;
        b["varId"] = varId;
        bindings.append(b);
    }
    obj["bindings"] = bindings;
    return obj;
}

void ProjectFileManager::applyItemProperties(CanvasItem *item, const QJsonObject &itemObj, QStringList *warnings) const
{
    const QJsonObject propsObj = itemObj.value("properties").toObject();
    for (auto it = propsObj.cbegin(); it != propsObj.cend(); ++it) {
        const QString key = it.key();
        const QVariant value = it.value().toVariant();
        if (value.isNull()) {
            if (warnings)
                warnings->append(QObject::tr("item %1 property %2 is null").arg(item->objectName(), key));
            continue;
        }
        item->setPropertyValue(key, value);
    }
}

void ProjectFileManager::applyBindings(CanvasItem *item,
                                       const QJsonArray &bindings,
                                       DataBindingManager *bindingMgr,
                                       QStringList *warnings) const
{
    if (!item || !bindingMgr)
        return;

    for (const QJsonValue &v : bindings) {
        if (!v.isObject())
            continue;
        const QJsonObject b = v.toObject();
        const QString varId = b.value("varId").toString().trimmed();
        const QString property = b.value("property").toString().trimmed();
        if (varId.isEmpty() || property.isEmpty())
            continue;

        const QString varProp = bindingPropertyToVarKey(property);
        if (!varProp.isEmpty()) {
            item->setPropertyValue(varProp, varId);
        } else {
            bindingMgr->bind(varId, item, property);
            if (warnings)
                warnings->append(QObject::tr("fallback direct bind: %1.%2 -> %3")
                                     .arg(item->objectName(), property, varId));
        }
    }
}

QString ProjectFileManager::ensureItemId(CanvasItem *item, int fallbackIndex) const
{
    if (!item)
        return QString();
    QString id = item->objectName().trimmed();
    if (id.isEmpty()) {
        id = QString("item_%1").arg(fallbackIndex);
        item->setObjectName(id);
    }
    return id;
}

QString ProjectFileManager::bindingPropertyToVarKey(const QString &property)
{
    if (property == "value")
        return "varId";

    static QRegularExpression valueRe("^value(\\d+)$");
    const auto m = valueRe.match(property);
    if (!m.hasMatch())
        return QString();

    return QString("varId%1").arg(m.captured(1));
}
