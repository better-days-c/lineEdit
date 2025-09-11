#ifndef PROJECTMANAGER_H
#define PROJECTMANAGER_H

#include "projectfile.h"
#include <QObject>
#include <QFileDialog>
#include <QMessageBox>

struct DesignPoint {
    QString line;
    double x;
    double y;
    int point;
};

class QWidget;

class ProjectManager : public QObject
{
    Q_OBJECT

public:
    explicit ProjectManager(QWidget* parent = nullptr);

    // 项目操作
    bool newProject();
    bool openProject();
    bool saveProject();
    bool saveProjectAs();
    void closeProject();

    // 获取当前项目
    const ProjectFile* currentProject() const { return m_currentProject; }
    ProjectFile* currentProject() { return m_currentProject; }

    // 项目状态
    bool hasProject() const { return m_currentProject != nullptr; }
    bool isProjectModified() const { return m_isModified; }

    // 数据文件操作
    QStringList getAbsoluteDataFilePaths() const;
    void addDataFile(const QString& filePath);
    void removeDataFile(const QString& filePath);

    void loadDesignTxt(const QString &filePath);

signals:
    void projectLoaded(const QStringList& dataFiles);
    void projectClosed();
    void projectModified();

private slots:
    void markModified();

private:
    QWidget* m_parent;
    ProjectFile* m_currentProject;
    QString m_currentProjectFile;
    bool m_isModified;

    QString getProjectFilter() const;
    bool confirmSaveChanges();
};

#endif // PROJECTMANAGER_H
