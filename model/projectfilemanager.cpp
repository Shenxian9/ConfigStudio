#include "projectfilemanager.h"

#include <QColor>
#include <QFile>
#include <QJsonDocument>
#include <QJsonParseError>

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

QJsonObject encodeTypedVariant(const QVariant &v)
{
    QJsonObject obj;
    switch (v.userType()) {
    case QMetaType::Bool:
        obj["type"] = "bool";
        obj["value"] = v.toBool();
        return obj;
    case QMetaType::Int:
    case QMetaType::LongLong:
    case QMetaType::UInt:
    case QMetaType::ULongLong:
        obj["type"] = "int";
        obj["value"] = QString::number(v.toLongLong());
        return obj;
    case QMetaType::Float:
    case QMetaType::Double:
        obj["type"] = "double";
        obj["value"] = v.toDouble();
        return obj;
    case QMetaType::QString:
        obj["type"] = "string";
        obj["value"] = v.toString();
        return obj;
    case QMetaType::QColor: {
        obj["type"] = "color";
        const QColor color = v.value<QColor>();
        obj["value"] = color.name(QColor::HexArgb);
        return obj;
    }
    default:
        break;
    }

    if (v.canConvert<QVariantMap>()) {
        obj["type"] = "map";
        obj["value"] = QJsonValue::fromVariant(v.toMap());
        return obj;
    }
    if (v.canConvert<QVariantList>()) {
        obj["type"] = "list";
        obj["value"] = QJsonValue::fromVariant(v.toList());
        return obj;
    }

    obj["type"] = "string";
    obj["value"] = v.toString();
    return obj;
}

QVariant decodeTypedVariant(const QJsonObject &obj)
{
    const QString type = obj.value("type").toString();
    const QJsonValue value = obj.value("value");

    if (type == "bool") return value.toBool();
    if (type == "int") {
        bool ok = false;
        const qlonglong n = value.toVariant().toLongLong(&ok);
        return ok ? QVariant::fromValue(n) : QVariant(value.toVariant());
    }
    if (type == "double") return value.toDouble();
    if (type == "color") return QColor(value.toString());
    if (type == "map" || type == "list") return value.toVariant();
    return value.toVariant();
}

} // namespace

bool ProjectFileManager::saveProject(const QString &filePath, const ProjectData &project, QString *errorText)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (errorText) *errorText = QObject::tr("Open file failed: %1").arg(file.errorString());
        return false;
    }

    const QJsonDocument doc(toJson(project));
    const qint64 written = file.write(doc.toJson(QJsonDocument::Indented));
    if (written <= 0) {
        if (errorText) *errorText = QObject::tr("Write file failed: %1").arg(file.errorString());
        return false;
    }
    return true;
}

bool ProjectFileManager::loadProject(const QString &filePath, ProjectData *project, QString *errorText)
{
    if (!project) {
        if (errorText) *errorText = QObject::tr("project output pointer is null");
        return false;
    }

    QFile file(filePath);
    if (!file.exists()) {
        if (errorText) *errorText = QObject::tr("File does not exist");
        return false;
    }
    if (!file.open(QIODevice::ReadOnly)) {
        if (errorText) *errorText = QObject::tr("Open file failed: %1").arg(file.errorString());
        return false;
    }

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        if (errorText) *errorText = QObject::tr("Invalid JSON: %1").arg(parseError.errorString());
        return false;
    }

    return fromJson(doc.object(), project, errorText);
}

QJsonObject ProjectFileManager::toJson(const ProjectData &project)
{
    QJsonObject root;
    root["version"] = project.version;

    QJsonObject canvas;
    canvas["width"] = project.canvasWidth;
    canvas["height"] = project.canvasHeight;

    QJsonArray items;
    for (const ProjectItemData &item : project.items) {
        QJsonObject itemObj;
        itemObj["id"] = item.id;
        itemObj["type"] = item.type;
        itemObj["x"] = item.x;
        itemObj["y"] = item.y;
        itemObj["width"] = item.width;
        itemObj["height"] = item.height;
        itemObj["z"] = item.z;
        itemObj["rotation"] = item.rotation;
        itemObj["visible"] = item.visible;

        QJsonObject props;
        for (auto it = item.properties.cbegin(); it != item.properties.cend(); ++it)
            props[it.key()] = variantToJson(it.value());
        itemObj["properties"] = props;

        QJsonArray bindings;
        for (const ProjectBindingData &binding : item.bindings) {
            QJsonObject bindObj;
            bindObj["property"] = binding.property;
            bindObj["varId"] = binding.varId;
            bindings.append(bindObj);
        }
        itemObj["bindings"] = bindings;

        items.append(itemObj);
    }

    canvas["items"] = items;
    root["canvas"] = canvas;

    QJsonArray vars;
    for (const Variable &var : project.variables)
        vars.append(variableToJson(var));
    root["variables"] = vars;

    root["modbus"] = modbusToJson(project.modbus);

    if (!project.uiState.isEmpty())
        root["uiState"] = project.uiState;

    return root;
}

bool ProjectFileManager::fromJson(const QJsonObject &obj, ProjectData *project, QString *errorText)
{
    ProjectData data;
    data.version = obj.value("version").toInt(1);
    if (data.version > kCurrentVersion) {
        if (errorText) *errorText = QObject::tr("Unsupported project version: %1").arg(data.version);
        return false;
    }

    const QJsonObject canvas = obj.value("canvas").toObject();
    data.canvasWidth = canvas.value("width").toInt();
    data.canvasHeight = canvas.value("height").toInt();

    const QJsonArray items = canvas.value("items").toArray();
    for (const QJsonValue &value : items) {
        if (!value.isObject())
            continue;
        const QJsonObject itemObj = value.toObject();
        ProjectItemData item;
        item.id = itemObj.value("id").toString();
        item.type = itemObj.value("type").toString();
        item.x = itemObj.value("x").toInt();
        item.y = itemObj.value("y").toInt();
        item.width = itemObj.value("width").toInt(120);
        item.height = itemObj.value("height").toInt(80);
        item.z = itemObj.value("z").toDouble(0.0);
        item.rotation = itemObj.value("rotation").toDouble(0.0);
        item.visible = itemObj.value("visible").toBool(true);

        const QJsonObject propsObj = itemObj.value("properties").toObject();
        for (auto pit = propsObj.begin(); pit != propsObj.end(); ++pit)
            item.properties.insert(pit.key(), jsonToVariant(pit.value()));

        const QJsonArray bindings = itemObj.value("bindings").toArray();
        for (const QJsonValue &bindingValue : bindings) {
            if (!bindingValue.isObject())
                continue;
            const QJsonObject bindingObj = bindingValue.toObject();
            ProjectBindingData binding;
            binding.property = bindingObj.value("property").toString();
            binding.varId = bindingObj.value("varId").toString();
            if (!binding.property.isEmpty() && !binding.varId.isEmpty())
                item.bindings.push_back(binding);
        }

        data.items.push_back(item);
    }

    const QJsonArray vars = obj.value("variables").toArray();
    for (const QJsonValue &value : vars) {
        if (value.isObject())
            data.variables.push_back(variableFromJson(value.toObject()));
    }

    if (obj.contains("modbus") && obj.value("modbus").isObject())
        data.modbus = modbusFromJson(obj.value("modbus").toObject());

    data.uiState = obj.value("uiState").toObject();

    *project = data;
    return true;
}

QJsonValue ProjectFileManager::variantToJson(const QVariant &v)
{
    return encodeTypedVariant(v);
}

QVariant ProjectFileManager::jsonToVariant(const QJsonValue &value)
{
    if (value.isObject()) {
        const QJsonObject obj = value.toObject();
        if (obj.contains("type") && obj.contains("value"))
            return decodeTypedVariant(obj);
    }
    return value.toVariant();
}

QJsonObject ProjectFileManager::variableToJson(const Variable &var)
{
    QJsonObject obj;
    obj["id"] = var.id;
    obj["name"] = var.name;
    obj["deviceId"] = var.deviceId;
    obj["type"] = var.type;
    obj["unit"] = var.unit;
    obj["value"] = variantToJson(var.value);
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

Variable ProjectFileManager::variableFromJson(const QJsonObject &obj)
{
    Variable var;
    var.id = obj.value("id").toString();
    var.name = obj.value("name").toString();
    var.deviceId = obj.value("deviceId").toString();
    var.type = obj.value("type").toString("float32");
    var.unit = obj.value("unit").toString();
    var.value = jsonToVariant(obj.value("value"));
    var.timestamp = obj.value("timestamp").toVariant().toLongLong();

    var.area = registerAreaFromString(obj.value("area").toString());
    var.address = obj.value("address").toInt();
    var.count = obj.value("count").toInt(1);
    var.bitOffset = obj.value("bitOffset").toInt();
    var.bitWidth = obj.value("bitWidth").toInt(16);
    var.scale = obj.value("scale").toDouble(1.0);
    var.readOnly = obj.value("readOnly").toBool(true);
    var.endianness = endiannessFromString(obj.value("endianness").toString());

    var.lowLimit = obj.value("lowLimit").toDouble();
    var.highLimit = obj.value("highLimit").toDouble();
    var.strategy = static_cast<DataStrategy>(obj.value("strategy").toInt(static_cast<int>(DataStrategy::None)));
    return var;
}

QJsonObject ProjectFileManager::modbusToJson(const SerialPortConfig &cfg)
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

SerialPortConfig ProjectFileManager::modbusFromJson(const QJsonObject &obj)
{
    SerialPortConfig cfg;
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

    const QString base64 = obj.value("frameTerminator").toString();
    if (!base64.isEmpty()) {
        const QByteArray decoded = QByteArray::fromBase64(base64.toLatin1());
        if (!decoded.isEmpty())
            cfg.frameTerminator = decoded;
    }
    return cfg;
}
