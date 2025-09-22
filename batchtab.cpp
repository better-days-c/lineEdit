#include "batchtab.h"
#include <QMessageBox>

BatchTab::BatchTab(int batchIndex, QWidget *parent, ProjectModel *projectModel)
    : QWidget(parent)
//    , m_dataPointData(data)
//    , m_fileName(fileName)
    , m_plotWidget(nullptr)
    , m_tableView(nullptr)
    , m_tableModel(nullptr)
    , m_batchIndex(batchIndex)
    , m_projectModel(nullptr)
{
    m_dataPointData = new DataPointData;
//    m_designLines = new QList<DesignLine>;
    setProjectModel(projectModel);
    setupUI();
    setupControlPanel();
    updateStatusInfo();
    onAltThresholdChanged(1);
}

void BatchTab::setupUI()
{
    // 创建主分割器（水平）
    m_mainSplitter = new QSplitter(Qt::Horizontal, this);

    // 创建绘图组件
    m_plotWidget = new PlotWidget(this);
    m_plotWidget->setBatchData(m_dataPointData);
    m_plotWidget->setDesignLinesFile(m_designLinesFile);

    // 创建右侧分割器（垂直）
    m_rightSplitter = new QSplitter(Qt::Vertical, this);

    // 创建表格视图
    m_tableView = new QTableView(this);
    m_tableModel = new DatTableModel(this);
    m_tableModel->setBatchData(m_dataPointData);
    m_tableView->setModel(m_tableModel);

    // 创建控制面板
    m_controlPanel = new QWidget(this);
    m_controlPanel->setMaximumWidth(400);
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
    connect(m_plotWidget, &PlotWidget::selectionChanged, this, &BatchTab::onSelectionChanged);
}

void BatchTab::setupControlPanel()
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
    m_showQualityCheck = new QCheckBox("飞行高度", this);
    m_showQualityCheck->setChecked(true);

    columnLayout->addWidget(m_showLineNumberCheck);
    columnLayout->addWidget(m_showPointNumberCheck);
    columnLayout->addWidget(m_showXCoordCheck);
    columnLayout->addWidget(m_showYCoordCheck);
    columnLayout->addWidget(m_showQualityCheck);

    // 连接列可见性信号
    connect(m_showLineNumberCheck, &QCheckBox::toggled, this, &BatchTab::onColumnVisibilityChanged);
    connect(m_showPointNumberCheck, &QCheckBox::toggled, this, &BatchTab::onColumnVisibilityChanged);
    connect(m_showXCoordCheck, &QCheckBox::toggled, this, &BatchTab::onColumnVisibilityChanged);
    connect(m_showYCoordCheck, &QCheckBox::toggled, this, &BatchTab::onColumnVisibilityChanged);
    connect(m_showQualityCheck, &QCheckBox::toggled, this, &BatchTab::onColumnVisibilityChanged);

    // 质量控制组
    m_qualityControlGroup = new QGroupBox("质量控制", this);
    QVBoxLayout *qualityLayout = new QVBoxLayout(m_qualityControlGroup);

    QVBoxLayout *thresholdLayout = new QVBoxLayout();
    thresholdLayout->addWidget(new QLabel("高度阈值:", this));
    QHBoxLayout *thresholdSpinButtonLayout = new QHBoxLayout();
    m_lowAltThresholdSpin = new QDoubleSpinBox(this);
    m_lowAltThresholdSpin->setRange(0.0, 200.0);
    m_lowAltThresholdSpin->setSingleStep(1.0);
    m_lowAltThresholdSpin->setValue(m_dataPointData->lowAltThreshold);
    m_lowAltThresholdSpin->setDecimals(2);
    thresholdSpinButtonLayout->addWidget(m_lowAltThresholdSpin);

    m_highAltThresholdSpin = new QDoubleSpinBox(this);
    m_highAltThresholdSpin->setRange(0.0, 200.0);
    m_highAltThresholdSpin->setSingleStep(1.0);
    m_highAltThresholdSpin->setValue(m_dataPointData->highAltThreshold);
    m_highAltThresholdSpin->setDecimals(2);
    thresholdSpinButtonLayout->addWidget(m_highAltThresholdSpin);

    m_setThresholdBtn = new QPushButton("设置阈值", this);
    thresholdSpinButtonLayout->addWidget(m_setThresholdBtn);

    thresholdLayout->addLayout(thresholdSpinButtonLayout);

//    m_deleteLowQualityBtn = new QPushButton("删除低质量点", this);

    qualityLayout->addLayout(thresholdLayout);
//    qualityLayout->addWidget(m_deleteLowQualityBtn);

//    connect(m_lowAltThresholdSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
//            this, &BatchTab::onAltThresholdChanged);
//    connect(m_highAltThresholdSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
//            this, &BatchTab::onAltThresholdChanged);
    connect(m_setThresholdBtn, &QPushButton::clicked, this, &BatchTab::onAltThresholdChanged);
//    connect(m_deleteLowQualityBtn, &QPushButton::clicked, this, &DatFileTab::deleteLowQualityPoints);

    // 选择控制组
    m_selectionControlGroup = new QGroupBox("选择操作", this);
    QVBoxLayout *selectionLayout = new QVBoxLayout(m_selectionControlGroup);

    QHBoxLayout *startEndSelectingLayout = new QHBoxLayout();
    m_startSelectingBtn = new QPushButton("开始选择", this);
    m_endSelectingBtn = new QPushButton("结束选择", this);
    m_endSelectingBtn->setEnabled(false);
    startEndSelectingLayout->addWidget(m_startSelectingBtn);
    startEndSelectingLayout->addWidget(m_endSelectingBtn);
    selectionLayout->addLayout(startEndSelectingLayout);

    m_clearSelectionBtn = new QPushButton("清除选择", this);
    m_applySelectionBtn = new QPushButton("应用选区", this);
//    m_invertSelectionBtn = new QPushButton("反转选择", this);

    selectionLayout->addWidget(m_clearSelectionBtn);
    selectionLayout->addWidget(m_applySelectionBtn);
//    selectionLayout->addWidget(m_invertSelectionBtn);

    connect(m_applySelectionBtn, &QPushButton::clicked, this, &BatchTab::applySelection);
    connect(m_clearSelectionBtn, &QPushButton::clicked, this, &BatchTab::clearSelection);
    connect(m_startSelectingBtn, &QPushButton::clicked, this, &BatchTab::startSelecting);
    connect(m_endSelectingBtn, &QPushButton::clicked, this, &BatchTab::endSelecting);

    // 状态信息
    QGroupBox *statusGroup = new QGroupBox("状态信息", this);
    QVBoxLayout *statusLayout = new QVBoxLayout(statusGroup);

    m_pointCountLabel = new QLabel(this);
    m_lineCountLabel = new QLabel(this);
    m_selectionCountLabel = new QLabel(this);
    m_selectedPointLabel = new QLabel(this);

    statusLayout->addWidget(m_pointCountLabel);
    statusLayout->addWidget(m_lineCountLabel);
    statusLayout->addWidget(m_selectionCountLabel);
    statusLayout->addWidget(m_selectedPointLabel);
    connect(m_plotWidget, &PlotWidget::pointClicked, this, &BatchTab::updateSelectedPoint);

    // 组装控制面板
    layout->addWidget(m_columnControlGroup);
    layout->addWidget(m_qualityControlGroup);
    layout->addWidget(m_selectionControlGroup);
    layout->addWidget(statusGroup);
    layout->addStretch();
}

void BatchTab::setProjectModel(ProjectModel *model) {
    m_projectModel = model;
    if (m_projectModel) {
        connect(m_projectModel, &ProjectModel::projectModified,
                this, [this]() { update(); });
    }
    QList<DataPoint> &points = m_projectModel->getBatches()[m_batchIndex].points;
    for (const DataPoint &point : points) {
        m_dataPointData->addPoint(point);
    }
    m_batchName = m_projectModel->getBatches()[m_batchIndex].batchName;
    m_designLinesFile = m_projectModel->getDesignLines();
}

void BatchTab::updateStatusInfo()
{
    if (!m_dataPointData)
        return;

    int visiblePoints = 0;
    for (const DataPoint &point : m_dataPointData->points) {
        if (point.isVisible) visiblePoints++;
    }

    m_pointCountLabel->setText(QString("可见点: %1 / %2")
                              .arg(visiblePoints)
                              .arg(m_dataPointData->points.size()));

    m_lineCountLabel->setText(QString("线条数: %1")
                             .arg(m_dataPointData->lineMap.size()));

    /// TODO: 更新选择计数
    m_selectionCountLabel->setText("选中点: 0");
}

void BatchTab::updateSelectedPoint(int index)
{
    if (!m_selectedPointLabel || index < 0 || index >= m_dataPointData->points.size()) {
        return;
    }

    const DataPoint& point = m_dataPointData->points[index];

    // 格式化状态栏信息
    QString info = QString("点号: %1 | 线号: %2")
                       .arg(point.fn)
                       .arg(point.lineId);

    m_selectedPointLabel->setText(info);
}

void BatchTab::onSelectionChanged()
{
    updateStatusInfo();
}

void BatchTab::onColumnVisibilityChanged()
{
    m_tableModel->setColumnVisible(DatTableModel::LineId, m_showLineNumberCheck->isChecked());
    m_tableModel->setColumnVisible(DatTableModel::FN, m_showPointNumberCheck->isChecked());
    m_tableModel->setColumnVisible(DatTableModel::X_Coordinate, m_showXCoordCheck->isChecked());
    m_tableModel->setColumnVisible(DatTableModel::Y_Coordinate, m_showYCoordCheck->isChecked());
    m_tableModel->setColumnVisible(DatTableModel::Alt, m_showQualityCheck->isChecked());
}

void BatchTab::onAltThresholdChanged(double value)
{
    Q_UNUSED(value);
    if (m_lowAltThresholdSpin->value() < m_highAltThresholdSpin->value()){
        m_dataPointData->setThreshold(m_lowAltThresholdSpin->value(), m_highAltThresholdSpin->value());
        updateStatusInfo();
        for (DataPoint &point: m_dataPointData->points) {
            point.isNormalAlt = m_dataPointData->isNormalAlt(point.alt);
        }
        m_plotWidget->invalidatePoints();
//        m_plotWidget->update();
    }
    else
        QMessageBox::warning(this, "错误", "无效的高度阈值设置！");
    qDebug() << "onALtThresholdChanged";
}

//void DatFileTab::deleteLowQualityPoints()
//{
//    m_datFileData->hideByOffset(m_datFileData->lowAltThreshold);
//    m_tableModel->refreshVisibleRows();
//    m_plotWidget->update();
//    updateStatusInfo();
//}

///已重写
void BatchTab::applySelection()
{
//    QVector<QRectF> regions = m_plotWidget->getSelectionRegions();
//    for (const QRectF &region : regions) {
//        m_datFileData->hideByRegion(region, false);
//    }
    m_dataPointData->hideByRegion(m_plotWidget->getSelection(), false);

    m_dataPointData->regenerateLineNumbers();
    m_tableModel->refreshVisibleRows();
    m_plotWidget->invalidatePoints();
//    m_plotWidget->update();
    updateStatusInfo();
    m_plotWidget->clearSelection();
}

void BatchTab::zoomToFit()
{
    m_plotWidget->zoomToFit();
}

void BatchTab::clearSelection()
{
    m_plotWidget->clearSelection();
}

QVector<int> BatchTab::getSelectedPointIndices() const
{
    // TODO: 实现获取选中点的功能
    return QVector<int>();
}

void BatchTab::deleteSelectedPoints()
{
    // TODO: 实现删除选中点的功能
}

void BatchTab::applyColumnMapping(const ColumnMapping &mapping)
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

void BatchTab::startSelecting()
{
    m_plotWidget->setClickMode(PlotWidget::Select);
    m_startSelectingBtn->setEnabled(false);
    m_endSelectingBtn->setEnabled(true);
}

void BatchTab::endSelecting()
{
    m_plotWidget->clearSelection();
    m_plotWidget->setClickMode(PlotWidget::Normal);
    m_endSelectingBtn->setEnabled(false);
    m_startSelectingBtn->setEnabled(true);
}
