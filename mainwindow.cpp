#include "mainwindow.h"
//#include "ui_MainWindow.h"
#include <QHeaderView>
#include <QMessageBox>
#include <QToolBar>
#include <QApplication>
#include "previewdialog.h"
#include <QInputDialog>
#include "batchtab.h"

///删除附加选区功能，优化反选功能
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
//    , ui(new Ui::MainWindow)
    , m_treeView(new ProjectTreeView(this))
    , m_tabWidget(nullptr)
//    , m_statusBar(nullptr)
    , m_progressBar(nullptr)
    , m_statusLabel(nullptr)
{
//    ui->setupUi(this);
    m_projectManager = new ProjectManager(this);
    connect(m_projectManager, &ProjectManager::projectNewed,
            this, &MainWindow::onProjectNewed);
    connect(m_projectManager, &ProjectManager::projectLoaded,
            this, &MainWindow::onProjectLoaded);
    connect(m_projectManager, &ProjectManager::projectClosed,
            this, &MainWindow::onProjectClosed);
    connect(m_projectManager, &ProjectManager::projectModified,
            this, &MainWindow::onProjectModified);

    setupUI();
    setupMenuBar();
    setupToolBar();
    setupStatusBar();
    createActions();

    setWindowTitle("测线编辑器");
    setMinimumSize(1024, 768);
}

MainWindow::~MainWindow()
{
//    delete ui;
    delete m_projectManager;
}

void MainWindow::setupUI()
{
    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setTabsClosable(true);
    m_tabWidget->setMovable(true);

    // 设置树视图
    m_treeView->setProjectModel(m_projectManager->currentProject());
    connect(m_treeView, &ProjectTreeView::designLineVisibilityChanged,
            this, &MainWindow::onDesignLineVisibilityChanged);
    connect(m_treeView, &ProjectTreeView::addBatchRequested,
            this, &MainWindow::onAddBatch);
    connect(m_treeView, &ProjectTreeView::deleteItemRequested,
            this, &MainWindow::onDeleteItem);
    connect(m_treeView, &ProjectTreeView::batchDoubleClicked,
            this, &MainWindow::onBatchDoubleClicked);
    connect(m_treeView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &MainWindow::onSelectionChanged);

    // 设置主分割器
    QSplitter* mainSplitter = new QSplitter(Qt::Horizontal, this);
    mainSplitter->addWidget(m_treeView);
    mainSplitter->addWidget(m_tabWidget);
    mainSplitter->setSizes(QList<int>() << 200 << 700);

    setCentralWidget(mainSplitter);

    connect(m_tabWidget, &QTabWidget::tabCloseRequested, this, &MainWindow::closeCurrentTab);
    connect(m_tabWidget, &QTabWidget::currentChanged, this, &MainWindow::onTabChanged);
//    setCentralWidget(m_tabWidget);
}

void MainWindow::setupMenuBar()
{
    QMenuBar *menuBar = this->menuBar();

    //项目菜单
    QMenu *projectMenu = menuBar->addMenu("项目");

    projectMenu->addAction("新建项目", m_projectManager, &ProjectManager::newProject);
    projectMenu->addAction("打开项目", m_projectManager, &ProjectManager::openProject);
    projectMenu->addSeparator();
    projectMenu->addAction("保存项目", m_projectManager, &ProjectManager::saveProject);
    projectMenu->addAction("另存为", m_projectManager, &ProjectManager::saveProjectAs);
    projectMenu->addSeparator();
    projectMenu->addAction("关闭项目", m_projectManager, &ProjectManager::closeProject);

    // 文件菜单
    QMenu *fileMenu = menuBar->addMenu("文件(&F)");

    m_openDesignLineAction = new QAction("打开设计线文件", this);
    m_openDesignLineAction->setEnabled(false); // 初始禁用，有项目后启用
    m_openDesignLineAction->setStatusTip("打开TXT文件");
    connect(m_openDesignLineAction, &QAction::triggered, this, &MainWindow::onOpenDesignLineFile);
    fileMenu->addAction(m_openDesignLineAction);

    m_openDesignLineAction = new QAction("打开设计线文件", this);
    m_openDesignLineAction->setEnabled(false); // 初始禁用，有项目后启用
    m_openDesignLineAction->setStatusTip("打开TXT文件");
    connect(m_openDesignLineAction, &QAction::triggered, this, &MainWindow::onOpenDesignLineFile);
    fileMenu->addAction(m_openDesignLineAction);

    fileMenu->addSeparator();

    m_addBatchAction = new QAction("添加架次", this);
    m_addBatchAction->setEnabled(false); // 初始禁用，有项目后启用
    connect(m_addBatchAction, &QAction::triggered, this, &MainWindow::onAddBatch);
    fileMenu->addAction(m_addBatchAction);


    m_openDataAction = new QAction("打开飞行数据文件", this);
    m_openDataAction->setEnabled(false); // 初始禁用，有架次后启用
    m_openDataAction->setStatusTip("打开DAT文件");
    connect(m_openDataAction, &QAction::triggered, this, &MainWindow::onOpenDataFile);
    fileMenu->addAction(m_openDataAction);

    fileMenu->addSeparator();

//    m_saveAction = new QAction("保存(&S)", this);
//    m_saveAction->setShortcut(QKeySequence::Save);
//    m_saveAction->setStatusTip("保存当前项目");
//    m_saveAction->setEnabled(false);
//    connect(m_saveAction, &QAction::triggered, this, &MainWindow::saveCurrentFile);
//    fileMenu->addAction(m_saveAction);

    m_deleteAction = new QAction("移除当前项", this);
    m_deleteAction->setEnabled(true);
    m_deleteAction->setStatusTip("从项目中移除当前文件/架次");
    connect(m_deleteAction, &QAction::triggered, this, &MainWindow::onDeleteItem);
    fileMenu->addAction(m_deleteAction);

    fileMenu->addSeparator();

    m_exportAction = new QAction("导出(&E)...", this);
    m_exportAction->setStatusTip("导出裁剪后的数据文件");
    m_exportAction->setEnabled(false);
    connect(m_exportAction, &QAction::triggered, this, &MainWindow::exportCurrentFile);
    fileMenu->addAction(m_exportAction);

    fileMenu->addSeparator();

    m_closeAction = new QAction("关闭标签页(&C)", this);
    m_closeAction->setShortcut(QKeySequence::Close);
    m_closeAction->setStatusTip("关闭当前标签页");
    m_closeAction->setEnabled(false);
    connect(m_closeAction, &QAction::triggered, this, &MainWindow::closeCurrentTab);
    fileMenu->addAction(m_closeAction);

    m_exitAction = new QAction("退出(&X)", this);
    m_exitAction->setShortcut(QKeySequence::Quit);
    m_exitAction->setStatusTip("退出程序");
    connect(m_exitAction, &QAction::triggered, this, &QWidget::close);
    fileMenu->addAction(m_exitAction);

    // 视图菜单
    QMenu *viewMenu = menuBar->addMenu("视图(&V)");

    m_zoomToFitAction = new QAction("缩放至适合(&F)", this);
    m_zoomToFitAction->setStatusTip("缩放到适合窗口大小");
//    m_zoomToFitAction->setEnabled(false);
    viewMenu->addAction(m_zoomToFitAction);

    m_clearSelectionAction = new QAction("清除选择(&C)", this);
    m_clearSelectionAction->setStatusTip("清除所有选择区域");
    m_clearSelectionAction->setEnabled(false);
    viewMenu->addAction(m_clearSelectionAction);
}

void MainWindow::setupToolBar()
{
    QToolBar *mainToolBar = addToolBar("主工具栏");

//    mainToolBar->addAction(m_openDesignLineAction);
//    mainToolBar->addAction(m_openDataAction);
//    mainToolBar->addAction(m_addBatchAction);
//    mainToolBar->addAction(m_saveAction);
    mainToolBar->addSeparator();
    mainToolBar->addAction(m_zoomToFitAction);
    mainToolBar->addAction(m_clearSelectionAction);
}

void MainWindow::setupStatusBar()
{
    m_statusBar = statusBar();

    m_statusLabel = new QLabel("就绪");
    m_statusBar->addWidget(m_statusLabel);

    m_progressBar = new QProgressBar();
    m_progressBar->hide();
    m_statusBar->addPermanentWidget(m_progressBar);
}

void MainWindow::createActions()
{
    // 连接缩放和选择动作到当前标签页
    connect(m_zoomToFitAction, &QAction::triggered, [this]() {
        BatchTab *currentTab = getCurrentTab();
        if (currentTab) {
            currentTab->zoomToFit();
        }
    });

    connect(m_clearSelectionAction, &QAction::triggered, [this]() {
        BatchTab *currentTab = getCurrentTab();
        if (currentTab) {
            currentTab->clearSelection();
        }
    });
}

void MainWindow::onOpenDataFile()
{
    QModelIndex batchIndex = m_treeView->getSelectedBatchIndex();
    if (!batchIndex.isValid()) return;

    int batchIdx = m_treeView->getBatchIndex(batchIndex);
    if (batchIdx < 0) return;

    QString filePath = QFileDialog::getOpenFileName(
        this,
        "打开DAT文件",
        "",
        "DAT文件 (*.dat);;所有文件 (*)"
    );

    if (filePath.isEmpty())
        return;

    // 检查文件是否符合行数约束
    bool result = m_projectManager->currentProject()->addDataFile(batchIdx, filePath);
    if (!result) {
        // 获取架次信息以显示正确的行数约束
        const auto& batches = m_projectManager->currentProject()->getBatches();
        if (batchIdx < batches.size() && batches[batchIdx].size > 0) {
//            QMessageBox::warning(this, "文件不匹配",
//                                QString("该文件行数: %2 与架次中其他文件不一致！\n"
//                                        "要求行数: %1\n").arg(batches[batchIdx].size));
        } else {
            QMessageBox::warning(this, "错误", "无法添加测试线文件！");
        }
    }

//    PreviewDialog previewDialog(fileName, this);
//    if (previewDialog.exec() != QDialog::Accepted) {
//        return; // 用户取消了配置
//    }
//    else{
//        m_statusLabel->setText("正在加载文件...");
//        m_progressBar->show();
//        m_progressBar->setRange(0, 0); // 不确定进度

//        QApplication::processEvents(); // 更新UI

//        BatchData *data = loadDatFileWithDialog(fileName, previewDialog);

//        m_progressBar->hide();

//        if (data) {
//            QFileInfo fileInfo(fileName);
//            DatFileTab *tab = new DatFileTab(data, fileInfo.baseName(), this);

//            //应用列映射配置到表格模型
//            ColumnMapping mapping = previewDialog.getColumnMapping();
//            tab->applyColumnMapping(mapping);

//            int index = m_tabWidget->addTab(tab, fileInfo.baseName());
//            m_tabWidget->setCurrentIndex(index);

//            m_statusLabel->setText(QString("已加载文件: %1 (%2 个点)")
//                                  .arg(fileInfo.baseName())
//                                  .arg(data->points.size()));

//            // 启用相关动作
//            m_saveAction->setEnabled(true);
//            m_exportAction->setEnabled(true);
//            m_closeAction->setEnabled(true);
//            m_zoomToFitAction->setEnabled(true);
//            m_clearSelectionAction->setEnabled(true);
//        } else {
//            m_statusLabel->setText("文件加载失败");
//            QMessageBox::warning(this, "错误", "无法加载文件: " + fileName);
//        }
    //    }
}

void MainWindow::onAddBatch()
{
    bool ok;
    QString batchName = QInputDialog::getText(
        this, "添加架次", "请输入架次名称:", QLineEdit::Normal, "", &ok
    );

    if (ok && !batchName.isEmpty()) {
        if (!m_projectManager->currentProject()->addBatch(batchName)) {
            QMessageBox::warning(this, "警告", "架次名称已存在！");
        }
    }
}

void MainWindow::onDeleteItem()
{
    QModelIndexList selection = m_treeView->selectionModel()->selectedIndexes();
    if (selection.isEmpty()) return;

    QModelIndex index = selection.first();

    if (m_treeView->isDesignLineItem(index)) {
        int designLineIndex = m_treeView->getDesignLineIndex(index);
        if (designLineIndex >= 0) {
            if (QMessageBox::question(this, "确认删除",
                                     "确定要删除这个设计线文件吗？") == QMessageBox::Yes) {
                m_projectManager->currentProject()->removeDesignLineFile(designLineIndex);
            }
        }
    } else if (m_treeView->isBatchItem(index)) {
        int batchIndex = m_treeView->getBatchIndex(index);
        if (batchIndex >= 0) {
            if (QMessageBox::question(this, "确认删除",
                                     "确定要删除这个架次吗？\n架次中的所有数据文件也将被删除。") == QMessageBox::Yes) {
                m_projectManager->currentProject()->removeBatch(batchIndex);
            }
        }
    } else if (m_treeView->isTestLineFileItem(index)) {
        int batchIndex = m_treeView->getBatchIndex(index);
        int fileIndex = m_treeView->getTestLineFileIndex(index);
        if (batchIndex >= 0 && fileIndex >= 0) {
            if (QMessageBox::question(this, "确认删除",
                                     "确定要删除这个数据文件吗？") == QMessageBox::Yes) {
                m_projectManager->currentProject()->removeDataFile(batchIndex, fileIndex);
            }
        }
    }
}

void MainWindow::onDesignLineVisibilityChanged(int index, bool visible)
{
    m_projectManager->currentProject()->setDesignLineVisibility(index, visible);
}

void MainWindow::onBatchDoubleClicked(int batchIndex)
{
    openTabWidget(batchIndex);
}

void MainWindow::onSelectionChanged()
{
    // 更新Action的启用状态
    QModelIndex batchIndex = m_treeView->getSelectedBatchIndex();
    m_openDataAction->setEnabled(batchIndex.isValid());
    m_exportAction->setEnabled(batchIndex.isValid());
}

void MainWindow::onOpenDesignLineFile()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "打开TXT文件",
        "",
        "TXT文件 (*.txt);;所有文件 (*)"
    );

    if (fileName.isEmpty())
        return;

    m_projectManager->loadDesignTxt(fileName);
}

void MainWindow::closeCurrentTab()
{
    int currentIndex = m_tabWidget->currentIndex();
    if (currentIndex >= 0) {
        BatchTab *tab = qobject_cast<BatchTab*>(m_tabWidget->widget(currentIndex));
        if (tab) {
            delete tab->getDataPointData(); // 清理数据
        }

        m_tabWidget->removeTab(currentIndex);

        if (m_tabWidget->count() == 0) {
            // 没有标签页时禁用相关动作
//            m_saveAction->setEnabled(false);
            m_exportAction->setEnabled(false);
            m_closeAction->setEnabled(false);
//            m_zoomToFitAction->setEnabled(false);
            m_clearSelectionAction->setEnabled(false);

            m_statusLabel->setText("就绪");
        }
    }
}

void MainWindow::onTabChanged(int index)
{
    if (index >= 0) {
        BatchTab *tab = getCurrentTab();
        if (tab) {
            DataPointData *data = tab->getDataPointData();
//            m_statusLabel->setText(QString("当前文件: %1 (%2 个点)")
//                                  .arg(tab->getFileName())
//                                  .arg(data->points.size()));
            QString batchName = m_projectManager->currentProject()->m_batches[tab->getBatchIndex()].batchName;
            m_statusLabel->setText(QString("当前架次: %1 (%2 个点）")
                                   .arg(batchName)
                                   .arg(data->points.size()));
        }
    }
}

void MainWindow::exportCurrentFile()
{
    BatchTab *currentTab = getCurrentTab();
    if (!currentTab)
        return;

//    QString fileName = QFileDialog::getSaveFileName(
//        this,
//        "导出数据",
//        currentTab->getBatchName() + "_export.csv",
//        "CSV文件 (*.csv);;DAT文件 (*.dat);;所有文件 (*)"
//    );

    QString selectedPath = QFileDialog::getExistingDirectory(
        nullptr,
        tr("选择导出文件夹路径"),
        QDir::homePath(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );

    if (selectedPath.isEmpty()) {
        // 用户取消了选择
        return;
    }

    // 用户输入文件夹名称
    bool ok;
    QString folderName = QInputDialog::getText(
        nullptr,
        tr("输入文件夹名称"),
        tr("请输入新文件夹的名称:"),
        QLineEdit::Normal,
        "exported_files",
        &ok
    );

    if (!ok || folderName.isEmpty()) {
        // 用户取消或未输入有效名称
        return;
    }

    // 创建目标文件夹
    QDir dir(selectedPath);
    QString newFolderPath = dir.filePath(folderName);
    if (!dir.exists(folderName)) {
        if (!dir.mkdir(folderName)) {
            QMessageBox::critical(
                nullptr,
                tr("错误"),
                tr("无法创建文件夹: %1").arg(newFolderPath)
            );
            return;
        }
    }

    Batch& currentBatch = currentTab->getBatch();

    for (int i = 0; i < currentBatch.filePaths.size(); i++) {
        QString& originalPath = currentBatch.filePaths[i];
        QFileInfo fileInfo(originalPath);
        QString originalName = fileInfo.baseName();
        QString newPath = QDir(newFolderPath).filePath(originalName + "_cut.dat");

        if (!writeDatFile(originalPath, newPath, currentBatch)) {
            QMessageBox::critical(
                nullptr,
                tr("错误"),
                tr("无法写入文件: %1").arg(newPath)
            );
            return;
        }
    }

    // 成功提示
    QMessageBox::information(
        nullptr,
        tr("成功"),
        tr("文件已成功导出到: %1").arg(newFolderPath)
                );
}

bool MainWindow::writeDatFile(const QString &originalPath, const QString &newPath, const Batch &currentBatch)
{
    QFile newfile(newPath);
    if (!newfile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    QTextStream out(&newfile);
    out.setCodec("UTF-8");
//    out << content;

    QFile originalFile(originalPath);
    if (!originalFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(nullptr, "错误", "无法打开文件 ");
        return false;
    }
    QTextStream in(&originalFile);
    in.setCodec("UTF-8");
    QStringList previewData;
    QString lastLine;

    // 跳过头部
    while (!in.atEnd()) {
        lastLine = in.readLine();
        out << lastLine;
        out << "\n";
        if (lastLine.trimmed().split(' ', QString::SkipEmptyParts)[0] == "LINE")
            break;
    }
    int i = 0;
    DataPoint point = currentBatch.points[i];
    while (!in.atEnd()){
        QString line = in.readLine();
        if (point.isVisible) {
            QString newLineId = point.lineId;
            line.replace(0, newLineId.size(), newLineId);
            out << line;
            out << "\n";
        }
        i++;
        if (i < currentBatch.points.size()) point = currentBatch.points[i];
    }
    originalFile.close();

    return true;
}

//dataPointData* MainWindow::loadDatFile(const QString &fileName, int skipLines)
//{
//    QFile file(fileName);
//    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
//        return nullptr;
//    }

//    dataPointData *data = new dataPointData();
//    data->fileName = QFileInfo(fileName).baseName();

//    QTextStream in(&file);
//    QString line;
//    int lineCount = 0;

//    while (!in.atEnd()) {
//        line = in.readLine();
//        lineCount++;

//        if(lineCount<=skipLines)continue;

//        // 解析数据行
//        QStringList parts = line.split(QRegExp("\\s+"), QString::SkipEmptyParts);
//        if (parts.size() < 5)
//            continue;

//        bool ok;
//        QString lineNumber = parts[0];
//        int pointNumber = parts[1].toInt(&ok);
//        if (!ok) continue;

//        double x = parts[2].toDouble(&ok);
//        if (!ok) continue;

//        double y = parts[3].toDouble(&ok);
//        if (!ok) continue;

//        double quality = parts[4].toDouble(&ok);
//        if (!ok) continue;

//        DataPoint point(lineNumber, pointNumber, x, y, quality);
//        data->addPoint(point);
//    }

//    if (data->points.isEmpty()) {
//        delete data;
//        return nullptr;
//    }

//    return data;
//}

BatchTab* MainWindow::getCurrentTab() const
{
    int currentIndex = m_tabWidget->currentIndex();
    if (currentIndex >= 0) {
        return qobject_cast<BatchTab*>(m_tabWidget->widget(currentIndex));
    }
    return nullptr;
}

void MainWindow::onProjectNewed() {
    m_tabWidget->clear();
    m_openDesignLineAction->setEnabled(true);
    m_addBatchAction->setEnabled(true);
    m_treeView->setProjectModel(m_projectManager->currentProject());
    connect(m_treeView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &MainWindow::onSelectionChanged);
}

void MainWindow::onProjectLoaded()
{
    // 更新窗口标题
    if (m_projectManager->hasProject()) {
        setWindowTitle(QString("%1 - 测线编辑器")
                      .arg(m_projectManager->currentProject()->projectName));
    }
    m_openDesignLineAction->setEnabled(true);
    m_addBatchAction->setEnabled(true);
    m_treeView->setProjectModel(m_projectManager->currentProject());
    connect(m_treeView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &MainWindow::onSelectionChanged);
}

void MainWindow::onProjectClosed()
{
    /// 关闭所有数据文件
    closeCurrentTab();
    setWindowTitle("测线编辑器");
    m_treeView->setProjectModel(m_projectManager->currentProject()); // 通知树视图更新模型
}

void MainWindow::onProjectModified()
{
    // 在标题栏显示修改标记
    if (m_projectManager->hasProject()) {
        setWindowTitle(QString("%1* - 测线编辑器")
                      .arg(m_projectManager->currentProject()->projectName));
    }
}

bool MainWindow::isTabWidgetOpen(int batchIndex) const {
    for (int i = 0; i < m_tabWidget->count(); ++i) {
//        PlotWidget* plotWidget = qobject_cast<PlotWidget*>(m_tabWidget->widget(i));
//        if (plotWidget && plotWidget->getBatchIndex() == batchIndex) {
//            return true;
//        }
        BatchTab* batchTab = qobject_cast<BatchTab*>(m_tabWidget->widget(i));
        if (batchTab && batchTab->getBatchIndex() == batchIndex) {
            return true;
        }
    }
    return false;
}

void MainWindow::openTabWidget(int batchIndex) {
    // 检查标签页是否已打开
    if (isTabWidgetOpen(batchIndex)) {
        // 切换到已打开的标签页
        for (int i = 0; i < m_tabWidget->count(); ++i) {
            BatchTab* tabWidget = qobject_cast<BatchTab*>(m_tabWidget->widget(i));
            if (tabWidget && tabWidget->getBatchIndex() == batchIndex) {
                m_tabWidget->setCurrentIndex(i);
                return;
            }
        }
    }

    // 创建新的tab
    const auto& batches = m_projectManager->currentProject()->getBatches();
    if (batchIndex < 0 || batchIndex >= batches.size()) return;
    BatchTab* tabWidget = new BatchTab(batchIndex, this, m_projectManager->currentProject());
//    tabWidget->setProjectModel(m_projectManager->currentProject());

    m_tabWidget->addTab(tabWidget, batches[batchIndex].batchName);
    m_tabWidget->setCurrentWidget(tabWidget);
}

//BatchData* MainWindow::loadDatFileWithDialog(const QString &fileName, const PreviewDialog &dialog)
//{
//    QFile file(fileName);
//    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
//        return nullptr;
//    }

//    BatchData *data = new BatchData();
//    data->fileName = QFileInfo(fileName).baseName();

//    QTextStream in(&file);
//    QString line;
//    int lineCount = 0;
//    int skipLines = dialog.getSkipLines();
//    ColumnMapping mapping = dialog.getColumnMapping();

//    // 跳过指定的行数
//    while (!in.atEnd() && lineCount < skipLines) {
//        in.readLine();
//        lineCount++;
//    }

//    // 跳过表头行
//    if (!in.atEnd()) {
//        in.readLine();
//        lineCount++;
//    }

//    while (!in.atEnd()){
//        line = in.readLine();
//        lineCount++;
//        //解析数据行
//        QStringList parts = line.split(QRegExp("\\s+"), QString::SkipEmptyParts);
//        if (parts.size() <= qMax(qMax(qMax(qMax(mapping.lineIdColumn, mapping.fnColumn),
//                                 mapping.xCoordinateColumn), mapping.yCoordinateColumn),
//                                 mapping.offsetColumn)) continue;

//        bool ok;
//        QString lineId = parts[mapping.lineIdColumn];

//        int fn = parts[mapping.fnColumn].toInt(&ok);
//        if (!ok) continue;

//        double x = parts[mapping.xCoordinateColumn].toDouble(&ok);
//        if (!ok) continue;

//        double y = parts[mapping.yCoordinateColumn].toDouble(&ok);
//        if (!ok) continue;

//        double quality = parts[mapping.offsetColumn].toDouble(&ok);
//        if (!ok) continue;

//        DataPoint point(lineId, fn, x, y, quality);
//        data->addPoint(point);

//    }

//    file.close();

//    if (data->points.isEmpty()){
//        delete data;
//        return nullptr;
//    }
//    return data;
//}

//DatFileData *MainWindow::loadDesignData(const QString &fileName)
//{
//    QFile file(fileName);
//    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
//        return nullptr;
//    }

//    DatFileData *data = new DatFileData();
//    data->fileName = QFileInfo(fileName).baseName();

//    QTextStream in(&file);
//    QString line;
//    int lineCount = 0;


//    // 跳过指定的行数
//    while (!in.atEnd() && lineCount < skipLines) {
//        in.readLine();
//        lineCount++;
//    }

//    // 跳过表头行
//    if (!in.atEnd()) {
//        in.readLine();
//        lineCount++;
//    }

//    while (!in.atEnd()){
//        line = in.readLine();
//        lineCount++;
//        //解析数据行
//        QStringList parts = line.split(QRegExp("\\s+"), QString::SkipEmptyParts);
//        if (parts.size() <= qMax(qMax(qMax(qMax(mapping.lineIdColumn, mapping.fnColumn),
//                                 mapping.xCoordinateColumn), mapping.yCoordinateColumn),
//                                 mapping.offsetColumn)) continue;

//        bool ok;
//        QString lineId = parts[mapping.lineIdColumn];

//        int fn = parts[mapping.fnColumn].toInt(&ok);
//        if (!ok) continue;

//        double x = parts[mapping.xCoordinateColumn].toDouble(&ok);
//        if (!ok) continue;

//        double y = parts[mapping.yCoordinateColumn].toDouble(&ok);
//        if (!ok) continue;

//        double quality = parts[mapping.offsetColumn].toDouble(&ok);
//        if (!ok) continue;

//        DataPoint point(lineId, fn, x, y, quality);
//        data->addPoint(point);

//    }

//    file.close();

//    if (data->points.isEmpty()){
//        delete data;
//        return nullptr;
//    }
//    return data;
//}


//#include "mainwindow.moc"
