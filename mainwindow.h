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

class DatFileTab;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void openFile();
//    void exportData();
    void closeCurrentTab();
    void onTabChanged(int index);
//    void saveCurrentFile();
    void exportCurrentFile();
//    void about();
    void onProjectModified();
    void onProjectLoaded(const QStringList& dataFiles);
    void onProjectClosed();

private:
    ProjectManager* m_projectManager;

    QTableView *m_tableView;
    QTabWidget *m_tabWidget;
    QStatusBar *m_statusBar;
    QProgressBar *m_progressBar;
    QLabel *m_statusLabel;

    // Actions
    QAction *m_openAction;
    QAction *m_openDesignAction;
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

    // 加载DAT文件
    DatFileData* loadDatFile(const QString &fileName, int skipLines);

    // 获取当前活动的标签页
    DatFileTab* getCurrentTab() const;

    DatFileData* loadDatFileWithDialog(const QString &fileName, const PreviewDialog &dialog);

    DatFileData* loadDesignData(const QString &fileName);
};

// 单个DAT文件的标签页
class DatFileTab : public QWidget
{
    Q_OBJECT

public:
    explicit DatFileTab(DatFileData *data, const QString &fileName, QWidget *parent = nullptr);

    DatFileData* getDatFileData() const { return m_datFileData; }
    QString getFileName() const { return m_fileName; }

    // 获取选中的点的索引
    QVector<int> getSelectedPointIndices() const;

    // 删除选中的点（视图层）
    void deleteSelectedPoints();

    // 删除低质量点
    void deleteLowQualityPoints();

    // 应用选区
    void applySelection();

    // 应用列映射配置
    void applyColumnMapping(const ColumnMapping &mapping);

public slots:
    void onSelectionChanged();
    void onColumnVisibilityChanged();
    void onQualityThresholdChanged(double threshold);
    void zoomToFit();
    void clearSelection();

private:
    void setupUI();
    void setupControlPanel();
    void updateStatusInfo();
    ColumnMapping m_columnMapping;  // 保存列映射信息

private:
    DatFileData *m_datFileData;
    QString m_fileName;

    // UI组件
    QSplitter *m_mainSplitter;
    QSplitter *m_rightSplitter;

    PlotWidget *m_plotWidget;
    QTableView *m_tableView;
    DatTableModel *m_tableModel;

    // 控制面板
    QWidget *m_controlPanel;
    QGroupBox *m_columnControlGroup;
    QGroupBox *m_qualityControlGroup;
    QGroupBox *m_selectionControlGroup;

    // 列可见性控制
    QCheckBox *m_showLineNumberCheck;
    QCheckBox *m_showPointNumberCheck;
    QCheckBox *m_showXCoordCheck;
    QCheckBox *m_showYCoordCheck;
    QCheckBox *m_showQualityCheck;

    // 质量控制
    QDoubleSpinBox *m_qualityThresholdSpin;
    QPushButton *m_deleteLowQualityBtn;

    // 选择控制
    QPushButton *m_applySelectionBtn;
    QPushButton *m_clearSelectionBtn;
    QPushButton *m_invertSelectionBtn;

    // 状态信息
    QLabel *m_pointCountLabel;
    QLabel *m_lineCountLabel;
    QLabel *m_selectionCountLabel;
};

#endif // MAINWINDOW_H
