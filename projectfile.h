#ifndef PROJECTFILE_H
#define PROJECTFILE_H

#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

class ProjectFile
{
public:
    ProjectFile();

    // 项目基本信息
    QString projectName;
    QString projectPath;
    QString description;
    QDateTime createdTime;
    QDateTime lastModified;

    // 数据文件列表
    QStringList dataFiles;

    // 设计线文件(json存储
    QJsonObject designObj;

    // 设计线列表
    QStringList designFiles;

    // 共有信息
    QJsonObject commonSettings;

    // 保存和加载项目文件
    bool saveProject(const QString& filePath);
    bool loadProject(const QString& filePath);

    // 文件管理
    void addDataFile(const QString& filePath);
    void removeDataFile(const QString& filePath);
    bool hasDataFile(const QString& filePath) const;
    bool hasDesignFile(const QString& filePath) const;

    // 共有信息管理
    void setCommonSetting(const QString& key, const QJsonValue& value);
    QJsonValue getCommonSetting(const QString& key) const;

    // 验证项目文件完整性
    bool validateProject() const;

private:
    QJsonObject toJson() const;
    void fromJson(const QJsonObject& json);
};

#endif // PROJECTFILE_H
