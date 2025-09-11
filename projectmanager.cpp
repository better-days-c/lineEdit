#include "projectmanager.h"
#include <QWidget>
#include <QDir>
#include <QInputDialog>

ProjectManager::ProjectManager(QWidget* parent)
    : QObject(parent)
    , m_parent(parent)
    , m_currentProject(nullptr)
    , m_isModified(false)
{
}

bool ProjectManager::newProject()
{
    if (!confirmSaveChanges()) {
        return false;
    }

    QString projectName = QInputDialog::getText(m_parent, "新建项目", "项目名称:");
    if (projectName.isEmpty()) {
        return false;
    }

    QString projectDir = QFileDialog::getExistingDirectory(m_parent, "选择项目目录");
    if (projectDir.isEmpty()) {
        return false;
    }

    closeProject();

    m_currentProject = new ProjectFile();
    m_currentProject->projectName = projectName;
    m_currentProject->projectPath = projectDir;

    m_currentProjectFile = QDir(projectDir).filePath(projectName + ".qproj");
    m_isModified = true;

    return true;
}

bool ProjectManager::openProject()
{
    if (!confirmSaveChanges()) {
        return false;
    }

    QString fileName = QFileDialog::getOpenFileName(
        m_parent,
        "打开项目",
        QString(),
        getProjectFilter()
    );

    if (fileName.isEmpty()) {
        return false;
    }

    closeProject();

    m_currentProject = new ProjectFile();
    if (m_currentProject->loadProject(fileName)) {
        m_currentProjectFile = fileName;
        m_isModified = false;

        // 发射信号，通知主窗口打开数据文件
        emit projectLoaded(getAbsoluteDataFilePaths());
        return true;
    } else {
        delete m_currentProject;
        m_currentProject = nullptr;
        QMessageBox::warning(m_parent, "错误", "无法加载项目文件！");
        return false;
    }
}

bool ProjectManager::saveProject()
{
    if (!m_currentProject) {
        return false;
    }

    if (m_currentProjectFile.isEmpty()) {
        return saveProjectAs();
    }

    if (m_currentProject->saveProject(m_currentProjectFile)) {
        m_isModified = false;
        return true;
    } else {
        QMessageBox::warning(m_parent, "错误", "无法保存项目文件！");
        return false;
    }
}

bool ProjectManager::saveProjectAs()
{
    if (!m_currentProject) {
        return false;
    }

    QString fileName = QFileDialog::getSaveFileName(
        m_parent,
        "保存项目",
        m_currentProject->projectName + ".qproj",
        getProjectFilter()
    );

    if (fileName.isEmpty()) {
        return false;
    }

    m_currentProjectFile = fileName;
    return saveProject();
}

void ProjectManager::closeProject()
{
    if (m_currentProject) {
        delete m_currentProject;
        m_currentProject = nullptr;
        m_currentProjectFile.clear();
        m_isModified = false;
        emit projectClosed();
    }
}

QStringList ProjectManager::getAbsoluteDataFilePaths() const
{
    if (!m_currentProject) {
        return QStringList();
    }

    QStringList absolutePaths;
    QDir projectDir(m_currentProject->projectPath);

    for (const QString& relativePath : m_currentProject->dataFiles) {
        if (QFileInfo(relativePath).isAbsolute()) {
            absolutePaths.append(relativePath);
        } else {
            absolutePaths.append(projectDir.absoluteFilePath(relativePath));
        }
    }

    return absolutePaths;
}

void ProjectManager::addDataFile(const QString& filePath)
{
    if (!m_currentProject) {
        return;
    }

    // 尝试使用相对路径
    QDir projectDir(m_currentProject->projectPath);
    QString relativePath = projectDir.relativeFilePath(filePath);

    // 如果相对路径包含 ".."，则使用绝对路径
    if (relativePath.startsWith("..")) {
        m_currentProject->addDataFile(filePath);
    } else {
        m_currentProject->addDataFile(relativePath);
    }

    markModified();
}

void ProjectManager::removeDataFile(const QString& filePath)
{
    if (!m_currentProject) {
        return;
    }

    QDir projectDir(m_currentProject->projectPath);
    QString relativePath = projectDir.relativeFilePath(filePath);

    m_currentProject->removeDataFile(filePath);
    m_currentProject->removeDataFile(relativePath);

    markModified();
}

void ProjectManager::markModified()
{
    if (!m_isModified) {
        m_isModified = true;
        emit projectModified();
    }
}

QString ProjectManager::getProjectFilter() const
{
    return "Qt项目文件 (*.qproj);;所有文件 (*.*)";
}

bool ProjectManager::confirmSaveChanges()
{
    if (m_isModified && m_currentProject) {
        QMessageBox::StandardButton ret = QMessageBox::question(
            m_parent,
            "保存更改",
            QString("项目 '%1' 已被修改。\n是否保存更改？")
                .arg(m_currentProject->projectName),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel
        );

        switch (ret) {
        case QMessageBox::Save:
            return saveProject();
        case QMessageBox::Discard:
            return true;
        case QMessageBox::Cancel:
        default:
            return false;
        }
    }
    return true;
}
