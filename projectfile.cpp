#include "projectfile.h"
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QDebug>

ProjectFile::ProjectFile()
{
    createdTime = QDateTime::currentDateTime();
    lastModified = createdTime;
}

bool ProjectFile::saveProject(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "无法保存项目文件:" << filePath;
        return false;
    }

    lastModified = QDateTime::currentDateTime();
    projectPath = QFileInfo(filePath).absolutePath();

    QJsonDocument doc(toJson());
    file.write(doc.toJson());

    return true;
}

bool ProjectFile::loadProject(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "无法打开项目文件:" << filePath;
        return false;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (doc.isNull() || !doc.isObject()) {
        qDebug() << "项目文件格式错误:" << filePath;
        return false;
    }

    fromJson(doc.object());
    projectPath = QFileInfo(filePath).absolutePath();

    return validateProject();
}

void ProjectFile::addDataFile(const QString& filePath)
{
    if (!dataFiles.contains(filePath)) {
        dataFiles.append(filePath);
    }
}

void ProjectFile::removeDataFile(const QString& filePath)
{
    dataFiles.removeAll(filePath);
}

bool ProjectFile::hasDataFile(const QString& filePath) const
{
    return dataFiles.contains(filePath);
}

void ProjectFile::setCommonSetting(const QString& key, const QJsonValue& value)
{
    commonSettings[key] = value;
}

QJsonValue ProjectFile::getCommonSetting(const QString& key) const
{
    return commonSettings.value(key);
}

bool ProjectFile::validateProject() const
{
    // 检查所有数据文件是否存在
    for (const QString& filePath : dataFiles) {
        QString absolutePath = filePath;
        if (QFileInfo(filePath).isRelative()) {
            absolutePath = QDir(projectPath).absoluteFilePath(filePath);
        }

        if (!QFile::exists(absolutePath)) {
            qDebug() << "数据文件不存在:" << absolutePath;
            return false;
        }
    }
    return true;
}

QJsonObject ProjectFile::toJson() const
{
    QJsonObject json;
    json["projectName"] = projectName;
    json["description"] = description;
    json["createdTime"] = createdTime.toString(Qt::ISODate);
    json["lastModified"] = lastModified.toString(Qt::ISODate);

    QJsonArray filesArray;
    for (const QString& file : dataFiles) {
        filesArray.append(file);
    }
    json["dataFiles"] = filesArray;

    json["commonSettings"] = commonSettings;

    return json;
}

void ProjectFile::fromJson(const QJsonObject& json)
{
    projectName = json["projectName"].toString();
    description = json["description"].toString();
    createdTime = QDateTime::fromString(json["createdTime"].toString(), Qt::ISODate);
    lastModified = QDateTime::fromString(json["lastModified"].toString(), Qt::ISODate);

    dataFiles.clear();
    QJsonArray filesArray = json["dataFiles"].toArray();
    for (const QJsonValue& value : filesArray) {
        dataFiles.append(value.toString());
    }

    commonSettings = json["commonSettings"].toObject();
}
