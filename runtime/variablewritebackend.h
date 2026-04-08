#ifndef VARIABLEWRITEBACKEND_H
#define VARIABLEWRITEBACKEND_H

#include <QString>
#include <QVariant>

class IVariableWriteBackend
{
public:
    virtual ~IVariableWriteBackend() = default;
    virtual bool writeEnabled() const = 0;
    virtual bool writeVariable(const QString &varId, const QVariant &value, QString *errorText) = 0;
};

#endif // VARIABLEWRITEBACKEND_H
