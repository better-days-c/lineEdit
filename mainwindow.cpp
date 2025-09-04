#include "mainwindow.h"
#include <QHeaderView>
#include <QMessageBox>
#include <QToolBar>
#include <QApplication>
#include "previewdialog.h"

///删除附加选区功能，优化反选功能
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_tabWidget(nullptr)
//    , m_statusBar(nullptr)
    , m_progressBar(nullptr)
    , m_statusLabel(nullptr)
{
    setupUI();
    setupMenuBar();
    setupToolBar();
    setupStatusBar();
    createActions();

    setWindowTitle("测线编辑器");
    resize(1200, 800);
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setTabsClosable(true);
    m_tabWidget->setMovable(true);

    connect(m_tabWidget, &QTabWidget::tabCloseRequested, this, &MainWindow::closeCurrentTab);
    connect(m_tabWidget, &QTabWidget::currentChanged, this, &MainWindow::onTabChanged);

    setCentralWidget(m_tabWidget);
}

void MainWindow::setupMenuBar()
{
    QMenuBar *menuBar = this->menuBar();

    // 文件菜单
    QMenu *fileMenu = menuBar->addMenu("文件(&F)");

    m_openAction = new QAction("打开数据文件(&O)...", this);
    m_openAction->setShortcut(QKeySequence::Open);
    m_openAction->setStatusTip("打开DAT文件");
    connect(m_openAction, &QAction::triggered, this, &MainWindow::openFile);
    fileMenu->addAction(m_openAction);

    m_openAction = new QAction("打开设计测线文件(&O)...", this);
    m_openAction->setStatusTip("打开TXT文件");
    connect(m_openDesignAction, &QAction::triggered, this, &MainWindow::openDesignFile);
    fileMenu->addAction(m_openDesignAction);

    fileMenu->addSeparator();

    m_saveAction = new QAction("保存(&S)", this);
    m_saveAction->setShortcut(QKeySequence::Save);
    m_saveAction->setStatusTip("保存当前文件");
    m_saveAction->setEnabled(false);
//    connect(m_saveAction, &QAction::triggered, this, &MainWindow::saveCurrentFile);
    fileMenu->addAction(m_saveAction);

    m_exportAction = new QAction("导出(&E)...", this);
    m_exportAction->setStatusTip("导出处理后的数据");
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
    m_zoomToFitAction->setEnabled(false);
    viewMenu->addAction(m_zoomToFitAction);

    m_clearSelectionAction = new QAction("清除选择(&C)", this);
    m_clearSelectionAction->setStatusTip("清除所有选择区域");
    m_clearSelectionAction->setEnabled(false);
    viewMenu->addAction(m_clearSelectionAction);
}

void MainWindow::setupToolBar()
{
    QToolBar *mainToolBar = addToolBar("主工具栏");

    mainToolBar->addAction(m_openAction);
    mainToolBar->addAction(m_openDesignAction);
    mainToolBar->addAction(m_saveAction);
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
        DatFileTab *currentTab = getCurrentTab();
        if (currentTab) {
            currentTab->zoomToFit();
        }
    });

    connect(m_clearSelectionAction, &QAction::triggered, [this]() {
        DatFileTab *currentTab = getCurrentTab();
        if (currentTab) {
            currentTab->clearSelection();
        }
    });
}

void MainWindow::openFile()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "打开DAT文件",
        "",
        "DAT文件 (*.dat);;所有文件 (*)"
    );

    if (fileName.isEmpty())
        return;

    PreviewDialog previewDialog(fileName, this);
    if (previewDialog.exec() != QDialog::Accepted) {
        return; // 用户取消了配置
    }
    else{
        m_statusLabel->setText("正在加载文件...");
        m_progressBar->show();
        m_progressBar->setRange(0, 0); // 不确定进度

        QApplication::processEvents(); // 更新UI

        DatFileData *data = loadDatFileWithDialog(fileName, previewDialog);

        m_progressBar->hide();

        if (data) {
            QFileInfo fileInfo(fileName);
            DatFileTab *tab = new DatFileTab(data, fileInfo.baseName(), this);

            //应用列映射配置到表格模型
            ColumnMapping mapping = previewDialog.getColumnMapping();
            tab->applyColumnMapping(mapping);

            int index = m_tabWidget->addTab(tab, fileInfo.baseName());
            m_tabWidget->setCurrentIndex(index);

            m_statusLabel->setText(QString("已加载文件: %1 (%2 个点)")
                                  .arg(fileInfo.baseName())
                                  .arg(data->points.size()));

            // 启用相关动作
            m_saveAction->setEnabled(true);
            m_exportAction->setEnabled(true);
            m_closeAction->setEnabled(true);
            m_zoomToFitAction->setEnabled(true);
            m_clearSelectionAction->setEnabled(true);
        } else {
            m_statusLabel->setText("文件加载失败");
            QMessageBox::warning(this, "错误", "无法加载文件: " + fileName);
        }
    }
}

void MainWindow::openDesignFile()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "打开TXT文件",
        "",
        "TXT文件 (*.txt);;所有文件 (*)"
    );

    if (fileName.isEmpty())
        return;

    m_statusLabel->setText("正在加载文件...");
    m_progressBar->show();
    m_progressBar->setRange(0, 0); // 不确定进度

    QApplication::processEvents(); // 更新UI

    DatFileData *data = loadDatFileWithDialog(fileName, previewDialog);

        m_progressBar->hide();

        if (data) {
            QFileInfo fileInfo(fileName);
            DatFileTab *tab = new DatFileTab(data, fileInfo.baseName(), this);

            //应用列映射配置到表格模型
            ColumnMapping mapping = previewDialog.getColumnMapping();
            tab->applyColumnMapping(mapping);

            int index = m_tabWidget->addTab(tab, fileInfo.baseName());
            m_tabWidget->setCurrentIndex(index);

            m_statusLabel->setText(QString("已加载文件: %1 (%2 个点)")
                                  .arg(fileInfo.baseName())
                                  .arg(data->points.size()));

            // 启用相关动作
            m_saveAction->setEnabled(true);
            m_exportAction->setEnabled(true);
            m_closeAction->setEnabled(true);
            m_zoomToFitAction->setEnabled(true);
            m_clearSelectionAction->setEnabled(true);
        } else {
            m_statusLabel->setText("文件加载失败");
            QMessageBox::warning(this, "错误", "无法加载文件: " + fileName);
        }
    }
}

DatFileData* MainWindow::loadDatFileWithDialog(const QString &fileName, const PreviewDialog &dialog)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        return nullptr;
    }

    DatFileData *data = new DatFileData();
    data->fileName = QFileInfo(fileName).baseName();

    QTextStream in(&file);
    QString line;
    int lineCount = 0;
    int skipLines = dialog.getSkipLines();
    ColumnMapping mapping = dialog.getColumnMapping();

    // 跳过指定的行数
    while (!in.atEnd() && lineCount < skipLines) {
        in.readLine();
        lineCount++;
    }

    // 跳过表头行
    if (!in.atEnd()) {
        in.readLine();
        lineCount++;
    }

    while (!in.atEnd()){
        line = in.readLine();
        lineCount++;
        //解析数据行
        QStringList parts = line.split(QRegExp("\\s+"), QString::SkipEmptyParts);
        if (parts.size() <= qMax(qMax(qMax(qMax(mapping.lineIdColumn, mapping.fnColumn),
                                 mapping.xCoordinateColumn), mapping.yCoordinateColumn),
                                 mapping.offsetColumn)) continue;

        bool ok;
        QString lineId = parts[mapping.lineIdColumn];

        int fn = parts[mapping.fnColumn].toInt(&ok);
        if (!ok) continue;

        double x = parts[mapping.xCoordinateColumn].toDouble(&ok);
        if (!ok) continue;

        double y = parts[mapping.yCoordinateColumn].toDouble(&ok);
        if (!ok) continue;

        double quality = parts[mapping.offsetColumn].toDouble(&ok);
        if (!ok) continue;

        DataPoint point(lineId, fn, x, y, quality);
        data->addPoint(point);

    }

    file.close();

    if (data->points.isEmpty()){
        delete data;
        return nullptr;
    }
    return data;
}

void MainWindow::closeCurrentTab()
{
    int currentIndex = m_tabWidget->currentIndex();
    if (currentIndex >= 0) {
        DatFileTab *tab = qobject_cast<DatFileTab*>(m_tabWidget->widget(currentIndex));
        if (tab) {
            delete tab->getDatFileData(); // 清理数据
        }

        m_tabWidget->removeTab(currentIndex);

        if (m_tabWidget->count() == 0) {
            // 没有标签页时禁用相关动作
            m_saveAction->setEnabled(false);
            m_exportAction->setEnabled(false);
            m_closeAction->setEnabled(false);
            m_zoomToFitAction->setEnabled(false);
            m_clearSelectionAction->setEnabled(false);

            m_statusLabel->setText("就绪");
        }
    }
}

void MainWindow::onTabChanged(int index)
{
    if (index >= 0) {
        DatFileTab *tab = getCurrentTab();
        if (tab) {
            DatFileData *data = tab->getDatFileData();
            m_statusLabel->setText(QString("当前文件: %1 (%2 个点)")
                                  .arg(tab->getFileName())
                                  .arg(data->points.size()));
        }
    }
}

void MainWindow::exportCurrentFile()
{
    DatFileTab *currentTab = getCurrentTab();
    if (!currentTab)
        return;

    QString fileName = QFileDialog::getSaveFileName(
        this,
        "导出数据",
        currentTab->getFileName() + "_export.csv",
        "CSV文件 (*.csv);;DAT文件 (*.dat);;所有文件 (*)"
    );

    if (fileName.isEmpty())
        return;

    /// TODO: 实现导出功能
    QMessageBox::information(this, "提示", "导出功能待实现");
}

DatFileData* MainWindow::loadDatFile(const QString &fileName, int skipLines)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return nullptr;
    }

    DatFileData *data = new DatFileData();
    data->fileName = QFileInfo(fileName).baseName();

    QTextStream in(&file);
    QString line;
    int lineCount = 0;

    while (!in.atEnd()) {
        line = in.readLine();
        lineCount++;

        if(lineCount<=skipLines)continue;

        // 解析数据行
        QStringList parts = line.split(QRegExp("\\s+"), QString::SkipEmptyParts);
        if (parts.size() < 5)
            continue;

        bool ok;
        QString lineNumber = parts[0];
        int pointNumber = parts[1].toInt(&ok);
        if (!ok) continue;

        double x = parts[2].toDouble(&ok);
        if (!ok) continue;

        double y = parts[3].toDouble(&ok);
        if (!ok) continue;

        double quality = parts[4].toDouble(&ok);
        if (!ok) continue;

        DataPoint point(lineNumber, pointNumber, x, y, quality);
        data->addPoint(point);
    }

    if (data->points.isEmpty()) {
        delete data;
        return nullptr;
    }

    return data;
}

DatFileTab* MainWindow::getCurrentTab() const
{
    int currentIndex = m_tabWidget->currentIndex();
    if (currentIndex >= 0) {
        return qobject_cast<DatFileTab*>(m_tabWidget->widget(currentIndex));
    }
    return nullptr;
}

// DatFileTab 实现
DatFileTab::DatFileTab(DatFileData *data, const QString &fileName, QWidget *parent)
    : QWidget(parent)
    , m_datFileData(data)
    , m_fileName(fileName)
    , m_plotWidget(nullptr)
    , m_tableView(nullptr)
    , m_tableModel(nullptr)
{
    setupUI();
    setupControlPanel();
    updateStatusInfo();
}

void DatFileTab::setupUI()
{
    // 创建主分割器（水平）
    m_mainSplitter = new QSplitter(Qt::Horizontal, this);

    // 创建绘图组件
    m_plotWidget = new PlotWidget(this);
    m_plotWidget->setDatFileData(m_datFileData);

    // 创建右侧分割器（垂直）
    m_rightSplitter = new QSplitter(Qt::Vertical, this);

    // 创建表格视图
    m_tableView = new QTableView(this);
    m_tableModel = new DatTableModel(this);
    m_tableModel->setDatFileData(m_datFileData);
    m_tableView->setModel(m_tableModel);

    // 创建控制面板
    m_controlPanel = new QWidget(this);
    m_controlPanel->setMaximumWidth(300);
    m_controlPanel->setMinimumWidth(250);

    // 组装UI
    m_rightSplitter->addWidget(m_tableView);
    m_rightSplitter->addWidget(m_controlPanel);
    m_rightSplitter->setStretchFactor(0, 1);
    m_rightSplitter->setStretchFactor(1, 0);

    m_mainSplitter->addWidget(m_plotWidget);
    m_mainSplitter->addWidget(m_rightSplitter);
    m_mainSplitter->setStretchFactor(0, 2);
    m_mainSplitter->setStretchFactor(1, 1);

    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->addWidget(m_mainSplitter);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // 连接信号
    connect(m_plotWidget, &PlotWidget::selectionChanged, this, &DatFileTab::onSelectionChanged);
}

void DatFileTab::setupControlPanel()
{
    QVBoxLayout *layout = new QVBoxLayout(m_controlPanel);

    // 列可见性控制组
    m_columnControlGroup = new QGroupBox("显示列", this);
    QVBoxLayout *columnLayout = new QVBoxLayout(m_columnControlGroup);

    m_showLineNumberCheck = new QCheckBox("线号", this);
    m_showLineNumberCheck->setChecked(true);
    m_showPointNumberCheck = new QCheckBox("点号", this);
    m_showPointNumberCheck->setChecked(true);
    m_showXCoordCheck = new QCheckBox("X坐标", this);
    m_showXCoordCheck->setChecked(true);
    m_showYCoordCheck = new QCheckBox("Y坐标", this);
    m_showYCoordCheck->setChecked(true);
    m_showQualityCheck = new QCheckBox("数据质量", this);
    m_showQualityCheck->setChecked(true);

    columnLayout->addWidget(m_showLineNumberCheck);
    columnLayout->addWidget(m_showPointNumberCheck);
    columnLayout->addWidget(m_showXCoordCheck);
    columnLayout->addWidget(m_showYCoordCheck);
    columnLayout->addWidget(m_showQualityCheck);

    // 连接列可见性信号
    connect(m_showLineNumberCheck, &QCheckBox::toggled, this, &DatFileTab::onColumnVisibilityChanged);
    connect(m_showPointNumberCheck, &QCheckBox::toggled, this, &DatFileTab::onColumnVisibilityChanged);
    connect(m_showXCoordCheck, &QCheckBox::toggled, this, &DatFileTab::onColumnVisibilityChanged);
    connect(m_showYCoordCheck, &QCheckBox::toggled, this, &DatFileTab::onColumnVisibilityChanged);
    connect(m_showQualityCheck, &QCheckBox::toggled, this, &DatFileTab::onColumnVisibilityChanged);

    // 质量控制组
    m_qualityControlGroup = new QGroupBox("质量控制", this);
    QVBoxLayout *qualityLayout = new QVBoxLayout(m_qualityControlGroup);

    QHBoxLayout *thresholdLayout = new QHBoxLayout();
    thresholdLayout->addWidget(new QLabel("质量阈值:", this));
    m_qualityThresholdSpin = new QDoubleSpinBox(this);
    m_qualityThresholdSpin->setRange(0.0, 100.0);
    m_qualityThresholdSpin->setSingleStep(0.1);
    m_qualityThresholdSpin->setValue(m_datFileData->offsetThreshold);
    m_qualityThresholdSpin->setDecimals(2);
    thresholdLayout->addWidget(m_qualityThresholdSpin);

    m_deleteLowQualityBtn = new QPushButton("删除低质量点", this);

    qualityLayout->addLayout(thresholdLayout);
    qualityLayout->addWidget(m_deleteLowQualityBtn);

    connect(m_qualityThresholdSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &DatFileTab::onQualityThresholdChanged);
    connect(m_deleteLowQualityBtn, &QPushButton::clicked, this, &DatFileTab::deleteLowQualityPoints);

    // 选择控制组
    m_selectionControlGroup = new QGroupBox("选择操作", this);
    QVBoxLayout *selectionLayout = new QVBoxLayout(m_selectionControlGroup);

    m_applySelectionBtn = new QPushButton("应用选区", this);
    m_clearSelectionBtn = new QPushButton("清除选择", this);
    m_invertSelectionBtn = new QPushButton("反转选择", this);

    selectionLayout->addWidget(m_applySelectionBtn);
    selectionLayout->addWidget(m_clearSelectionBtn);
    selectionLayout->addWidget(m_invertSelectionBtn);

    connect(m_applySelectionBtn, &QPushButton::clicked, this, &DatFileTab::applySelection);
    connect(m_clearSelectionBtn, &QPushButton::clicked, this, &DatFileTab::clearSelection);

    // 状态信息
    QGroupBox *statusGroup = new QGroupBox("状态信息", this);
    QVBoxLayout *statusLayout = new QVBoxLayout(statusGroup);

    m_pointCountLabel = new QLabel(this);
    m_lineCountLabel = new QLabel(this);
    m_selectionCountLabel = new QLabel(this);

    statusLayout->addWidget(m_pointCountLabel);
    statusLayout->addWidget(m_lineCountLabel);
    statusLayout->addWidget(m_selectionCountLabel);

    // 组装控制面板
    layout->addWidget(m_columnControlGroup);
    layout->addWidget(m_qualityControlGroup);
    layout->addWidget(m_selectionControlGroup);
    layout->addWidget(statusGroup);
    layout->addStretch();
}

void DatFileTab::updateStatusInfo()
{
    if (!m_datFileData)
        return;

    int visiblePoints = 0;
    for (const DataPoint &point : m_datFileData->points) {
        if (point.isVisible) visiblePoints++;
    }

    m_pointCountLabel->setText(QString("可见点: %1 / %2")
                              .arg(visiblePoints)
                              .arg(m_datFileData->points.size()));

    m_lineCountLabel->setText(QString("线条数: %1")
                             .arg(m_datFileData->lineMap.size()));

    /// TODO: 更新选择计数
    m_selectionCountLabel->setText("选中点: 0");
}

void DatFileTab::onSelectionChanged()
{
    updateStatusInfo();
}

void DatFileTab::onColumnVisibilityChanged()
{
    m_tableModel->setColumnVisible(DatTableModel::LineId, m_showLineNumberCheck->isChecked());
    m_tableModel->setColumnVisible(DatTableModel::FN, m_showPointNumberCheck->isChecked());
    m_tableModel->setColumnVisible(DatTableModel::X_Coordinate, m_showXCoordCheck->isChecked());
    m_tableModel->setColumnVisible(DatTableModel::Y_Coordinate, m_showYCoordCheck->isChecked());
    m_tableModel->setColumnVisible(DatTableModel::Offset, m_showQualityCheck->isChecked());
}

void DatFileTab::onQualityThresholdChanged(double threshold)
{
    m_datFileData->setThreshold(threshold);
    m_plotWidget->update();
    updateStatusInfo();
}

void DatFileTab::deleteLowQualityPoints()
{
    m_datFileData->hideByOffset(m_datFileData->offsetThreshold);
    m_tableModel->refreshVisibleRows();
    m_plotWidget->update();
    updateStatusInfo();
}

///已重写
void DatFileTab::applySelection()
{
//    QVector<QRectF> regions = m_plotWidget->getSelectionRegions();
//    for (const QRectF &region : regions) {
//        m_datFileData->hideByRegion(region, false);
//    }
    m_datFileData->hideByRegion(m_plotWidget->getSelection(), false);

    m_datFileData->regenerateLineNumbers();
    m_tableModel->refreshVisibleRows();
    m_plotWidget->update();
    updateStatusInfo();
}

void DatFileTab::zoomToFit()
{
    m_plotWidget->zoomToFit();
}

void DatFileTab::clearSelection()
{
    m_plotWidget->clearSelection();
}

QVector<int> DatFileTab::getSelectedPointIndices() const
{
    // TODO: 实现获取选中点的功能
    return QVector<int>();
}

void DatFileTab::deleteSelectedPoints()
{
    // TODO: 实现删除选中点的功能
}

void DatFileTab::applyColumnMapping(const ColumnMapping &mapping)
{
    m_columnMapping = mapping;

    // 根据映射配置设置列的可见性
    // 首先隐藏所有列
    m_showLineNumberCheck->setChecked(false);
    m_showPointNumberCheck->setChecked(false);
    m_showXCoordCheck->setChecked(false);
    m_showYCoordCheck->setChecked(false);
    m_showQualityCheck->setChecked(false);

    // 然后显示映射的列
    if (mapping.lineIdColumn >= 0) {
        m_showLineNumberCheck->setChecked(true);
    }
    if (mapping.fnColumn >= 0) {
        m_showPointNumberCheck->setChecked(true);
    }
    if (mapping.xCoordinateColumn >= 0) {
        m_showXCoordCheck->setChecked(true);
    }
    if (mapping.yCoordinateColumn >= 0) {
        m_showYCoordCheck->setChecked(true);
    }
    if (mapping.offsetColumn >= 0) {
        m_showQualityCheck->setChecked(true);
    }

    // 应用列可见性变化
    onColumnVisibilityChanged();

    // 更新状态信息
    updateStatusInfo();
}

//#include "mainwindow.moc"
