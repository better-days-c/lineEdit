#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableView>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMenuBar>
#include <QAction>
#include <QFileDialog>
#include <QLabel>
#include <QProgressBar>
#include <QSplitter>
#include <QGroupBox>
#include <QCheckBox>
#include <QStatusBar>

#include "tablemodel.h"
#include "previewdialog.h"
#include "datastructures.h"
#include "dattablemodel.h"
#include "plotwidget.h"
#include "projectmanager.h"
#include "projecttreeview.h"

class BatchTab;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
//    void exportData();
    void closeCurrentTab();
    void onTabChanged(int index);
//    void saveCurrentFile();
    void exportCurrentFile();
//    void about();
    void onProjectNewed();
    void onProjectModified();
    void onProjectLoaded();
    void onProjectClosed();

    // 菜单动作
    void onOpenDesignLineFile();
    void onOpenDataFile();
    void onAddBatch();
    void onDeleteItem();

    // 树视图相关
    void onDesignLineVisibilityChanged(int index, bool visible);
    void onBatchDoubleClicked(int batchIndex);
    void onSelectionChanged();

    /// 项目文件操作，由projectmanager统一管理
//    void onNewProject();
//    void onOpenProject();
//    void onSaveProject();
//    void onSaveAsProject();

private:
    Ui::MainWindow *ui;
    ProjectManager* m_projectManager;
    ProjectTreeView* m_treeView;

    QTableView *m_tableView;
    QTabWidget *m_tabWidget;
    QStatusBar *m_statusBar;
    QProgressBar *m_progressBar;
    QLabel *m_statusLabel;

    // Actions
    QAction *m_openDataAction;
    QAction *m_openDesignLineAction;
    QAction *m_addBatchAction;
    QAction *m_deleteAction;
    QAction *m_closeAction;
    QAction *m_saveAction;
    QAction *m_exportAction;
    QAction *m_exitAction;
    QAction *m_aboutAction;
    QAction *m_zoomToFitAction;
    QAction *m_clearSelectionAction;

    void createActions();
    void setupUI();
    void setupMenuBar();
    void setupToolBar();
    void setupStatusBar();

    bool isTabWidgetOpen(int batchIndex) const;
    void openTabWidget(int batchIndex);

    // 加载DAT文件
    DataPointData* loadDatFile(const QString &fileName, int skipLines);

    // 获取当前活动的标签页
    BatchTab* getCurrentTab() const;

//    BatchData* loadDatFileWithDialog(const QString &fileName, const PreviewDialog &dialog);

    DataPointData* loadDesignData(const QString &fileName);
};


#endif // MAINWINDOW_H
