#ifndef PROJECTSTORAGEMANAGER_H
#define PROJECTSTORAGEMANAGER_H

#pragma once

#include <QObject>
#include <QDateTime>
#include <QJsonObject>

struct ProjectFileInfo {
    QString filePath;
    QString fileName;
    QString baseName;
    QDateTime lastModified;
    qint64 sizeBytes = 0;
};

class ProjectStorageManager : public QObject
{
    Q_OBJECT
public:
    explicit ProjectStorageManager(QObject *parent = nullptr);

    QString projectRootDir() const;
    void setProjectRootDir(const QString &path);
    bool ensureProjectDir(QString *err = nullptr) const;

    QString normalizeProjectName(const QString &projectName) const;
    bool validateProjectName(const QString &projectName, QString *err = nullptr) const;
    QString makeProjectFilePath(const QString &projectName) const;

    QList<ProjectFileInfo> listProjects(QString *err = nullptr) const;
    QString projectNameFromPath(const QString &filePath) const;

    bool saveJsonToFile(const QString &filePath, const QJsonObject &root, QString *err = nullptr) const;
    bool loadJsonFromFile(const QString &filePath, QJsonObject *root, QString *err = nullptr) const;

    bool removeProject(const QString &filePath, QString *err = nullptr) const;
    bool renameProject(const QString &oldPath, const QString &newName, QString *newPath = nullptr, QString *err = nullptr) const;

private:
    QString m_projectRootDir;
};

#endif // PROJECTSTORAGEMANAGER_H
