#include "projectstoragemanager.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFileInfoList>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QRegularExpression>

ProjectStorageManager::ProjectStorageManager(QObject *parent)
    : QObject(parent)
    , m_projectRootDir(QStringLiteral("/userdata/ConfigStuidoProjects"))
{
}

QString ProjectStorageManager::projectRootDir() const
{
    return m_projectRootDir;
}

void ProjectStorageManager::setProjectRootDir(const QString &path)
{
    const QString normalized = path.trimmed();
    if (normalized.isEmpty())
        return;
    m_projectRootDir = normalized;
}

bool ProjectStorageManager::ensureProjectDir(QString *err) const
{
    QDir dir(projectRootDir());
    if (dir.exists())
        return true;

    if (!QDir().mkpath(projectRootDir())) {
        if (err) *err = tr("Create project directory failed: %1").arg(projectRootDir());
        return false;
    }
    return true;
}

QString ProjectStorageManager::normalizeProjectName(const QString &projectName) const
{
    return projectName.trimmed();
}

bool ProjectStorageManager::validateProjectName(const QString &projectName, QString *err) const
{
    const QString name = normalizeProjectName(projectName);
    if (name.isEmpty()) {
        if (err) *err = tr("Project name cannot be empty.");
        return false;
    }

    static const QRegularExpression badChars(QStringLiteral("[\\\\/:*?\"<>|]"));
    if (name.contains(badChars)) {
        if (err) *err = tr("Project name contains invalid characters: / \\ : * ? \" < > |");
        return false;
    }
    if (name == "." || name == "..") {
        if (err) *err = tr("Project name is invalid.");
        return false;
    }
    return true;
}

QString ProjectStorageManager::makeProjectFilePath(const QString &projectName) const
{
    QString fileName = normalizeProjectName(projectName);
    if (!fileName.endsWith(".cstudio", Qt::CaseInsensitive))
        fileName += ".cstudio";
    return QDir(projectRootDir()).filePath(fileName);
}

QList<ProjectFileInfo> ProjectStorageManager::listProjects(QString *err) const
{
    QList<ProjectFileInfo> out;
    if (!ensureProjectDir(err))
        return out;

    QDir dir(projectRootDir());
    QFileInfoList infos = dir.entryInfoList({"*.cstudio"}, QDir::Files | QDir::Readable, QDir::Time | QDir::Reversed);
    std::sort(infos.begin(), infos.end(), [](const QFileInfo &a, const QFileInfo &b) {
        return a.lastModified() > b.lastModified();
    });

    for (const QFileInfo &info : infos) {
        ProjectFileInfo f;
        f.filePath = info.absoluteFilePath();
        f.fileName = info.fileName();
        f.baseName = info.completeBaseName();
        f.lastModified = info.lastModified();
        f.sizeBytes = info.size();
        out.push_back(f);
    }
    return out;
}

QString ProjectStorageManager::projectNameFromPath(const QString &filePath) const
{
    return QFileInfo(filePath).completeBaseName();
}

bool ProjectStorageManager::saveJsonToFile(const QString &filePath, const QJsonObject &root, QString *err) const
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (err) *err = tr("Open file failed: %1").arg(file.errorString());
        return false;
    }

    const qint64 written = file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    if (written <= 0) {
        if (err) *err = tr("Write file failed: %1").arg(file.errorString());
        return false;
    }
    return true;
}

bool ProjectStorageManager::loadJsonFromFile(const QString &filePath, QJsonObject *root, QString *err) const
{
    if (!root) {
        if (err) *err = tr("root pointer is null");
        return false;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        if (err) *err = tr("Open file failed: %1").arg(file.errorString());
        return false;
    }

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        if (err) *err = tr("Invalid JSON: %1").arg(parseError.errorString());
        return false;
    }

    *root = doc.object();
    return true;
}

bool ProjectStorageManager::removeProject(const QString &filePath, QString *err) const
{
    if (!QFile::exists(filePath)) {
        if (err) *err = tr("Project file does not exist");
        return false;
    }
    if (!QFile::remove(filePath)) {
        if (err) *err = tr("Remove project failed");
        return false;
    }
    return true;
}

bool ProjectStorageManager::renameProject(const QString &oldPath, const QString &newName, QString *newPath, QString *err) const
{
    if (!validateProjectName(newName, err))
        return false;

    const QString targetPath = makeProjectFilePath(newName);
    if (QFile::exists(targetPath)) {
        if (err) *err = tr("Target project already exists");
        return false;
    }
    if (!QFile::rename(oldPath, targetPath)) {
        if (err) *err = tr("Rename project failed");
        return false;
    }
    if (newPath)
        *newPath = targetPath;
    return true;
}
