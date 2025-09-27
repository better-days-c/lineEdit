#ifndef BATCHTAB_H
#define BATCHTAB_H
#include <QWidget>
#include "datastructures.h"
#include "previewdialog.h"
#include "plotwidget.h"
#include "dattablemodel.h"
#include <QCheckBox>
#include "projectmodel.h"


// 单个DAT文件的标签页
class BatchTab : public QWidget
{
    Q_OBJECT

public:
    explicit BatchTab(int batchIndex, QWidget *parent = nullptr, ProjectModel* projectModel = nullptr);

    DataPointData* getDataPointData() const { return m_dataPointData; }
    QString getBatchName() const { return m_batchName; }
    Batch& getBatch() const { return m_projectModel->getBatches()[m_batchIndex]; }

    ///
    void setProjectModel(ProjectModel* model);
    int getBatchIndex() const { return m_batchIndex; }
    ///

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

    void startSelecting();
    void endSelecting();

    ///为单个线段匹配线号
    void matchLineNumber();

    double distanceBetweenPoints(double x1, double y1, double x2, double y2);

    double pointToLineSegmentDistance(const QPointF& point, const DesignLine& line);

    std::pair<DesignLine*, double> findClosestLineWithDistance(const QPointF& point, QList<DesignLine>& lines);

    void syncModel();

public slots:
    void onSelectionChanged();
    void onColumnVisibilityChanged();
    void onAltThresholdChanged(double value);
    void zoomToFit();
    void clearSelection();
    void resetDataPoints();
    void updateSelectedPoint(int index);
    void onChangeLineId(QString originalLineId, QString newLineId);
    void applyFnCut();

private:
    void setupUI();
    void setupControlPanel();
    void updateStatusInfo();
    ColumnMapping m_columnMapping;  // 保存列映射信息

private:
    DataPointData *m_dataPointData;
//    QList<DesignLineFile>& m_designLinesFile;
//    QList<DesignLine> *m_designLines;
    QString m_batchName;

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
    QDoubleSpinBox *m_lowAltThresholdSpin;
    QDoubleSpinBox *m_highAltThresholdSpin;
//    QPushButton *m_deleteLowQualityBtn;
    QPushButton *m_setThresholdBtn;

    // 选择控制
    QPushButton *m_applySelectionBtn;
    QPushButton *m_clearSelectionBtn;
//    QPushButton *m_invertSelectionBtn;
    QPushButton *m_startSelectingBtn;
    QPushButton *m_endSelectingBtn;

    QSpinBox *m_startFnSpin;
    QSpinBox *m_endFnSpin;
    QPushButton *m_applyFnCutBtn;

    QPushButton *m_assignLineNumberBtn;

    // 状态信息
    QLabel *m_pointCountLabel;
    QLabel *m_lineCountLabel;
    QLabel *m_selectionCountLabel;
    QLabel *m_selectedPointLabel;

    ///
    int m_batchIndex;
    ProjectModel* m_projectModel;
};

#endif // BATCHTAB_H
