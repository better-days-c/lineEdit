#ifndef PROJECTFILE_H
#define PROJECTFILE_H

#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <datastructures.h>

// 设计线文件结构
struct DesignLineFile {
    QString filePath;
    QList<DesignLine> data;
    bool visible; // 控制是否可见

    QJsonObject toJson() const;
    static DesignLineFile fromJson(const QJsonObject& json);
};

// 架次结构
struct Batch {
    QString batchName;
    QList<QString> filePaths;
    QList<DataPoint> points;
    QList<QString> relatedLines;
    int size = 0; // 记录文件行数约束

    QJsonObject toJson() const;
    static Batch fromJson(const QJsonObject& json);
};

class ProjectModel: public QObject
{
    Q_OBJECT

public:
    explicit ProjectModel(QObject *parent = nullptr);

    // 项目基本信息
    QString projectName;
    QString projectPath;
    QDateTime createdTime;
    QDateTime lastModified;

    /// 设计线文件(json存储
    QJsonObject designObj;

    QList<DesignLineFile> m_designLinesFile;
    QList<Batch> m_batches;

    // 保存和加载项目文件
    bool saveProject(const QString& filePath);
    bool loadProject(const QString& filePath);
    QString getProjectPath() const { return projectPath; }

    // 设计线相关操作
    bool addDesignLineFile(const QString& filePath);
    bool removeDesignLineFile(int index);
    QList<DesignLineFile>& getDesignLines() { return m_designLinesFile; }
    void setDesignLineVisibility(int index, bool visible);

    // 测试架次相关操作
    bool addBatch(const QString& batchName);
    bool removeBatch(int index);
    QList<Batch>& getBatches() { return m_batches; }

    // 测试线文件相关操作
    bool addDataFile(int batchIndex, const QString& filePath);
    bool removeDataFile(int batchIndex, int fileIndex);
    int getSize(const QString& filePath); // 获取文件行数

    // 项目基本信息
    QString getProjectName() const { return projectName; }
    void setProjectName(const QString& name) { projectName = name; }
    QDateTime getCreatedTime() const { return createdTime; }
    QDateTime getLastModifiedTime() const { return lastModified; }

//    // 文件管理
//    void addDataFile(const QString& filePath);
//    void removeDataFile(const QString& filePath);
//    bool hasDataFile(const QString& filePath) const;
//    bool hasDesignFile(const QString& filePath) const;

    // 验证项目文件完整性
    bool validateProject() const;

signals:
    void projectModified();
    void designLinesChanged();
    void batchesChanged();


private:
//    QJsonObject toJson() const;
//    void fromJson(const QJsonObject& json);
};

#endif // PROJECTFILE_H
