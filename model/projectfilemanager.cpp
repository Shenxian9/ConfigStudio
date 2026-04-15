#include "projectfilemanager.h"

#include <QColor>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonValue>

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

RegisterArea registerAreaFromString(const QString &value)
{
    if (value.compare("Coil", Qt::CaseInsensitive) == 0)
        return RegisterArea::Coil;
    if (value.compare("DiscreteInput", Qt::CaseInsensitive) == 0)
        return RegisterArea::DiscreteInput;
    if (value.compare("InputRegister", Qt::CaseInsensitive) == 0)
        return RegisterArea::InputRegister;
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

Endianness endiannessFromString(const QString &value)
{
    if (value.compare("BigEndianWordSwap", Qt::CaseInsensitive) == 0)
        return Endianness::BigEndianWordSwap;
    if (value.compare("LittleEndian", Qt::CaseInsensitive) == 0)
        return Endianness::LittleEndian;
    if (value.compare("LittleEndianByteSwap", Qt::CaseInsensitive) == 0)
        return Endianness::LittleEndianByteSwap;
    return Endianness::BigEndian;
}
}

QJsonObject ProjectFileManager::serializeVariable(const Variable &var)
{
    QJsonObject obj;
    obj["id"] = var.id;
    obj["name"] = var.name;
    obj["deviceId"] = var.deviceId;
    obj["type"] = var.type;
    obj["unit"] = var.unit;
    obj["value"] = variantToJsonValue(var.value);
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

bool ProjectFileManager::deserializeVariable(const QJsonObject &obj, Variable *var)
{
    if (!var)
        return false;
    var->id = obj.value("id").toString();
    var->name = obj.value("name").toString();
    var->deviceId = obj.value("deviceId").toString();
    var->type = obj.value("type").toString("float32");
    var->unit = obj.value("unit").toString();
    var->value = jsonValueToVariant(obj.value("value"));
    var->timestamp = obj.value("timestamp").toString().toLongLong();
    var->area = registerAreaFromString(obj.value("area").toString("HoldingRegister"));
    var->address = obj.value("address").toInt();
    var->count = qMax(1, obj.value("count").toInt(1));
    var->bitOffset = qMax(0, obj.value("bitOffset").toInt(0));
    var->bitWidth = qMax(1, obj.value("bitWidth").toInt(16));
    var->scale = obj.value("scale").toDouble(1.0);
    if (var->scale <= 0.0)
        var->scale = 1.0;
    var->readOnly = obj.value("readOnly").toBool(true);
    var->endianness = endiannessFromString(obj.value("endianness").toString("BigEndian"));
    var->lowLimit = obj.value("lowLimit").toDouble();
    var->highLimit = obj.value("highLimit").toDouble();
    var->strategy = static_cast<DataStrategy>(obj.value("strategy").toInt(static_cast<int>(DataStrategy::None)));
    return !var->id.trimmed().isEmpty();
}

QJsonObject ProjectFileManager::serializeSerialConfig(const SerialPortConfig &cfg)
{
    QJsonObject obj;
    obj["deviceId"] = cfg.deviceId;
    obj["portName"] = cfg.portName;
    obj["baudRate"] = static_cast<int>(cfg.baudRate);
    obj["dataBits"] = static_cast<int>(cfg.dataBits);
    obj["parity"] = static_cast<int>(cfg.parity);
    obj["stopBits"] = static_cast<int>(cfg.stopBits);
    obj["flowControl"] = static_cast<int>(cfg.flowControl);
    obj["slaveId"] = cfg.slaveId;
    obj["timeoutMs"] = cfg.timeoutMs;
    obj["retryCount"] = cfg.retryCount;
    obj["pollIntervalMs"] = cfg.pollIntervalMs;
    obj["defaultFunctionCode"] = cfg.defaultFunctionCode;
    obj["frameTerminator"] = QString::fromLatin1(cfg.frameTerminator.toBase64());
    return obj;
}

bool ProjectFileManager::deserializeSerialConfig(const QJsonObject &obj, SerialPortConfig *cfg)
{
    if (!cfg)
        return false;
    cfg->deviceId = obj.value("deviceId").toString("default");
    cfg->portName = obj.value("portName").toString("/dev/ttyS2");
    cfg->baudRate = obj.value("baudRate").toInt(QSerialPort::Baud9600);
    cfg->dataBits = static_cast<QSerialPort::DataBits>(obj.value("dataBits").toInt(QSerialPort::Data8));
    cfg->parity = static_cast<QSerialPort::Parity>(obj.value("parity").toInt(QSerialPort::NoParity));
    cfg->stopBits = static_cast<QSerialPort::StopBits>(obj.value("stopBits").toInt(QSerialPort::OneStop));
    cfg->flowControl = static_cast<QSerialPort::FlowControl>(obj.value("flowControl").toInt(QSerialPort::NoFlowControl));
    cfg->slaveId = obj.value("slaveId").toInt(1);
    cfg->timeoutMs = obj.value("timeoutMs").toInt(1000);
    cfg->retryCount = obj.value("retryCount").toInt(3);
    cfg->pollIntervalMs = obj.value("pollIntervalMs").toInt(500);
    cfg->defaultFunctionCode = obj.value("defaultFunctionCode").toInt(3);
    const QByteArray terminator = QByteArray::fromBase64(obj.value("frameTerminator").toString().toLatin1());
    if (!terminator.isEmpty())
        cfg->frameTerminator = terminator;
    return true;
}

QJsonObject ProjectFileManager::serializeCanvasItem(const ProjectCanvasItemData &item)
{
    QJsonObject obj;
    obj["id"] = item.id;
    obj["type"] = item.type;
    obj["x"] = item.geometry.x();
    obj["y"] = item.geometry.y();
    obj["width"] = item.geometry.width();
    obj["height"] = item.geometry.height();
    obj["z"] = item.zValue;
    obj["rotation"] = item.rotation;
    obj["visible"] = item.visible;
    obj["properties"] = serializeVariantMap(item.properties);

    QJsonArray bindingsArray;
    for (const ProjectBindingData &binding : item.bindings) {
        QJsonObject bindingObj;
        bindingObj["itemId"] = binding.itemId;
        bindingObj["property"] = binding.property;
        bindingObj["varId"] = binding.varId;
        bindingsArray.push_back(bindingObj);
    }
    obj["bindings"] = bindingsArray;
    return obj;
}

bool ProjectFileManager::deserializeCanvasItem(const QJsonObject &obj, ProjectCanvasItemData *item)
{
    if (!item)
        return false;
    item->id = obj.value("id").toString();
    item->type = obj.value("type").toString();
    item->geometry = QRect(obj.value("x").toInt(),
                           obj.value("y").toInt(),
                           qMax(20, obj.value("width").toInt()),
                           qMax(20, obj.value("height").toInt()));
    item->zValue = obj.value("z").toInt(0);
    item->rotation = obj.value("rotation").toDouble(0.0);
    item->visible = obj.value("visible").toBool(true);
    item->properties = deserializeVariantMap(obj.value("properties").toObject());
    item->bindings.clear();

    const QJsonArray bindings = obj.value("bindings").toArray();
    for (const QJsonValue &bindingValue : bindings) {
        const QJsonObject bindingObj = bindingValue.toObject();
        ProjectBindingData binding;
        binding.itemId = bindingObj.value("itemId").toString(item->id);
        binding.property = bindingObj.value("property").toString();
        binding.varId = bindingObj.value("varId").toString();
        if (binding.itemId.isEmpty() || binding.property.isEmpty() || binding.varId.isEmpty())
            continue;
        item->bindings.push_back(binding);
    }
    return !item->type.isEmpty();
}

QJsonObject ProjectFileManager::serializeVariantMap(const QVariantMap &map)
{
    QJsonObject obj;
    for (auto it = map.cbegin(); it != map.cend(); ++it)
        obj.insert(it.key(), variantToJsonValue(it.value()));
    return obj;
}

QVariantMap ProjectFileManager::deserializeVariantMap(const QJsonObject &obj)
{
    QVariantMap map;
    for (auto it = obj.begin(); it != obj.end(); ++it)
        map.insert(it.key(), jsonValueToVariant(it.value()));
    return map;
}

QJsonValue ProjectFileManager::variantToJsonValue(const QVariant &value)
{
    if (!value.isValid())
        return QJsonValue();

    if (value.userType() == QMetaType::QColor) {
        QJsonObject typed;
        typed["$type"] = "color";
        typed["value"] = value.value<QColor>().name();
        return typed;
    }

    if (value.type() == QVariant::DateTime) {
        QJsonObject typed;
        typed["$type"] = "datetime";
        typed["value"] = value.toDateTime().toString(Qt::ISODateWithMs);
        return typed;
    }

    return QJsonValue::fromVariant(value);
}

QVariant ProjectFileManager::jsonValueToVariant(const QJsonValue &value)
{
    if (value.isObject()) {
        const QJsonObject obj = value.toObject();
        const QString type = obj.value("$type").toString();
        if (type == "color")
            return QColor(obj.value("value").toString());
        if (type == "datetime")
            return QDateTime::fromString(obj.value("value").toString(), Qt::ISODateWithMs);
    }
    return value.toVariant();
}
