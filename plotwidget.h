#ifndef PLOTWIDGET_H
#define PLOTWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QRubberBand>
#include <QTimer>
#include "datastructures.h"
#include <QDebug>
#include <QPolygonF>
#include <QVector>

class PolygonSelectionWidget;

class PlotWidget : public QWidget
{
    Q_OBJECT

public:
    enum SelectionMode {
        Normal,      // 普通选择
        Add,         // 增加选区（Shift键）
        Subtract     // 减少选区（Ctrl键）
    };

    explicit PlotWidget(QWidget *parent = nullptr);

    // 设置数据
    void setBatchData(DataPointData *data);

//    // 获取选择区域
//    QVector<QRectF> getSelectionRegions() const { return m_selectionRegions; }

    // 获取当前选区多边形
    QPolygonF getSelection() const;

    // 解除选择
    void clearSelection();

    // 缩放到适合大小
    void zoomToFit();

signals:
    void selectionChanged();
    void pointsSelected(const QVector<int>& indices);
    void selectionCompleted(const QPolygonF &polygon);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

private slots:
//    void onRubberBandChanged(const QRect &selection);

private:
    DataPointData *m_datFileData;

    // 视图变换
    QPointF m_offset;           // 偏移
    double m_scale;             // 缩放比例
    QRectF m_dataRect;          // 数据范围

    // 选择功能
    QRubberBand *m_rubberBand;
    QPoint m_rubberBandOrigin;
//    QVector<QRectF> m_selectionRegions;  // 选择区域列表
    SelectionMode m_selectionMode;

    // 绘制相关
    QColor m_normalAltColor;   // 低偏航点颜色
    QColor m_abnormalAltColor;    // 高偏航点颜色
    QColor m_selectionColor;     // 选择区域颜色

    QVector<QPointF> m_vertices; //多边形顶点
    QPoint m_currentPoint;  //当前鼠标位置
    bool m_isSelecting; //是否正在选择
    QPolygonF m_selectionPolygon_screen;    //完成的多边形选区
    QPolygonF m_selectionPolygon_world;


    // 辅助函数
    QPointF worldToScreen(const QPointF &worldPoint) const;
    QPointF screenToWorld(const QPointF &screenPoint) const;
    QRectF screenToWorld(const QRectF &screenRect) const;

    void updateDataRect();
    void drawLines(QPainter &painter);
    void drawPoints(QPainter &painter);
//    void drawSelectionRegions(QPainter &painter);
    void drawGrid(QPainter &painter);

    // 获取线段（考虑质量分段）
    QVector<LineSegment> getQualitySegmentedLines() const;

    // 检查点是否在选择区域内
    bool isPointSelected(const QPointF &point) const;

    // 更新选择模式
    void updateSelectionMode();
};


//class PolygonSelectionWidget : public QWidget
//{
//    Q_OBJECT

//public:
//    explicit PolygonSelectionWidget(QWidget *parent = nullptr);

//    // 清除多边形选区
//    void clearSelection();

//    // 获取当前选区多边形
//    QPolygonF getSelection() const;

//    // 清除选区内的内容（需要根据你的具体需求实现）
//    void clearSelectedArea();

//signals:
//    void selectionCompleted(const QPolygonF &polygon);

//protected:
//    void paintEvent(QPaintEvent *event) override;
//    void mousePressEvent(QMouseEvent *event) override;
//    void mouseMoveEvent(QMouseEvent *event) override;
//    void mouseDoubleClickEvent(QMouseEvent *event) override;

//private:
//    QVector<QPoint> m_points;      // 多边形顶点
//    QPoint m_currentPoint;         // 当前鼠标位置
//    bool m_isSelecting;            // 是否正在选择
//    QPolygonF m_selectionPolygon;   // 完成的多边形选区
//};

#endif // PLOTWIDGET_H
