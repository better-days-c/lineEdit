#include "plotwidget.h"
#include <QPainter>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QApplication>
#include <QRubberBand>
#include <qmath.h>

PlotWidget::PlotWidget(QWidget *parent)
    : QWidget(parent)
    , m_dataPointData(nullptr)
    , m_offset(0, 0)
    , m_scale(1.0)
    , m_rubberBand(nullptr)
    , m_selectionMode(Normal)
    , m_normalAltColor(Qt::blue)
    , m_abnormalAltColor(Qt::red)
    , m_selectionColor(QColor(255, 255, 0, 100))
    , m_lineSegmentColor(Qt::darkGray)
    , m_isSelecting(false)
{
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    m_rubberBand = new QRubberBand(QRubberBand::Rectangle, this);
}

void PlotWidget::setBatchData(DataPointData *data)
{
    qDebug() << "setBatchData called.";
    m_dataPointData = data;
    if (data) {
        updateDataRect();
        zoomToFit();
    }
    update();
}

QPolygonF PlotWidget::getSelection() const
{
    qDebug() << "getSelection called.";
    return m_selectionPolygon_world;
}


// 解除选择
void PlotWidget::clearSelection()
{
    qDebug() << "clearSelection called.";
//    m_selectionRegions.clear();
    m_vertices.clear();
    m_selectionPolygon_world.clear();
    m_selectionPolygon_screen.clear();
    m_isSelecting = false;
    emit selectionChanged();
    update();
}

// 根据图件尺寸和窗口尺寸计算自动缩放比例
void PlotWidget::zoomToFit()
{
    qDebug() << "zoomToFit called.";
    if (!m_dataPointData || m_dataPointData->points.isEmpty())
        return;

    updateDataRect();

    QSize widgetSize = size();
    if (widgetSize.width() <= 0 || widgetSize.height() <= 0)
        return;

    double scaleX = widgetSize.width() / m_dataRect.width();
    double scaleY = widgetSize.height() / m_dataRect.height();

    m_scale = qMin(scaleX, scaleY) * 0.9; // 留一些边距

    // 居中
    QPointF center = m_dataRect.center();
    m_offset = QPointF(widgetSize.width() / 2.0, widgetSize.height() / 2.0) -
               QPointF(center.x() * m_scale, center.y() * m_scale);
    update();
}

void PlotWidget::paintEvent(QPaintEvent *event)
{
    qDebug() << "paintEvent called.";
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 绘制背景
    painter.fillRect(rect(), Qt::white);

    if (!m_dataPointData)
        return;

    // 绘制网格
    drawGrid(painter);

    // 绘制线条
//    drawLines(painter);

    // 绘制点
    drawPoints(painter);

    // 绘制选择区域
//    drawSelectionRegions(painter);

    // 绘制多边形选区
    if (!m_vertices.isEmpty()) {
        painter.setPen(QPen(Qt::blue, 2, Qt::DotLine));
        painter.setBrush(QBrush(QColor(100, 100, 255, 50))); // 半透明填充

        // 绘制已确定的线段
        for (int i = 1; i < m_vertices.size(); ++i) {
            painter.drawLine(m_vertices[i-1], m_vertices[i]);
        }

        // 绘制当前正在绘制的线段
        if (m_isSelecting && !m_vertices.isEmpty()) {
            painter.drawLine(m_vertices.last(), m_currentPoint);
        }

        // 如果有多边形完成，绘制填充
        if (!m_selectionPolygon_screen.isEmpty()) {
            painter.drawPolygon(m_selectionPolygon_screen);
        }
    }
}

void PlotWidget::mousePressEvent(QMouseEvent *event)
{
    qDebug() << "mousePressEvent called.";
    if (event->button() == Qt::LeftButton) {
        updateSelectionMode();

//        m_rubberBandOrigin = event->pos();
//        m_rubberBand->setGeometry(QRect(m_rubberBandOrigin, QSize()));
//        m_rubberBand->show();
//        m_isSelecting = true;
        if (!m_isSelecting) {
            // 开始新的选区
            m_vertices.clear();
            m_selectionPolygon_screen.clear();
            m_selectionPolygon_world.clear();
            m_isSelecting = true;
        }

        m_vertices.append(event->pos());
        update();
    }
    else if (event->button() == Qt::RightButton && m_isSelecting) {
        // 右键取消选区
        clearSelection();
    }

    QWidget::mousePressEvent(event);
}

void PlotWidget::mouseMoveEvent(QMouseEvent *event)
{
//    qDebug() << "mouseMoveEvent called.";
//    if (m_isSelecting && m_rubberBand->isVisible()) {
//        QRect selection(m_rubberBandOrigin, event->pos());
//        m_rubberBand->setGeometry(selection.normalized());
//    }
    if (m_isSelecting) {
        m_currentPoint = event->pos();
        update();
    }


    QWidget::mouseMoveEvent(event);
}

void PlotWidget::mouseReleaseEvent(QMouseEvent *event)
{
    qDebug() << "mouseReleaseEvent called.";
//    if (event->button() == Qt::LeftButton && m_isSelecting) {
//        m_isSelecting = false;

//        QRect selectionRect = m_rubberBand->geometry();
//        m_rubberBand->hide();

//        if (selectionRect.width() > 5 && selectionRect.height() > 5) {
//            QRectF worldSelection = screenToWorld(QRectF(selectionRect));

//            switch (m_selectionMode) {
//            case Normal:
//                m_selectionRegions.clear();
//                m_selectionRegions.append(worldSelection);
//                break;
//            case Add:
//                m_selectionRegions.append(worldSelection);
//                break;
//            case Subtract:
//                /// 从现有选区中减去新选区（简化实现，实际应该做布尔运算）
//                /// TODO: 实现更复杂的选区运算
//                break;
//            }

//            emit selectionChanged();
//            update();
//        }
//    }

    QWidget::mouseReleaseEvent(event);
}

void PlotWidget::wheelEvent(QWheelEvent *event)
{
    qDebug() << "wheelEvent called.";
    // 缩放功能
    const double scaleFactor = 1.15;
    QPointF mousePos = event->posF();
    QPointF oldWorldPos = screenToWorld(mousePos);

    if (event->angleDelta().y() > 0) {
        m_scale *= scaleFactor;

    } else {
        m_scale /= scaleFactor;
    }
    m_offset = mousePos - oldWorldPos * m_scale;

    // 以鼠标位置为中心缩放
//    m_offset += mousePos - newScreenPos;

    update();
}

void PlotWidget::keyPressEvent(QKeyEvent *event)
{
    qDebug() << "keyPressEvent called.";
    updateSelectionMode();
    QWidget::keyPressEvent(event);
}

void PlotWidget::keyReleaseEvent(QKeyEvent *event)
{
    qDebug() << "keyReleaseEvent called.";
    updateSelectionMode();
    QWidget::keyReleaseEvent(event);
}

void PlotWidget::resizeEvent(QResizeEvent *event)
{
    qDebug() << "resizeEvent called.";
    Q_UNUSED(event);
    zoomToFit();
}


void PlotWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    qDebug() << "mouseDoubleClickEvent called.";
    if (event->button() == Qt::LeftButton && m_isSelecting && m_vertices.size() >= 3) {
        //双击完成多边形

        m_selectionPolygon_screen = QPolygonF(m_vertices);
        for(QPointF &point : m_vertices)
        {
            point = screenToWorld(point);
        }
        m_selectionPolygon_world = QPolygonF(m_vertices);
        m_isSelecting = false;
        emit selectionCompleted(m_selectionPolygon_screen);
        update();
    }
}

//根据两个控制浮点数将真实坐标转换成窗口坐标
QPointF PlotWidget::worldToScreen(const QPointF &worldPoint) const
{
    qDebug() << "worldToScreen called.";
    qDebug() << worldPoint;
    return QPointF(worldPoint.x() * m_scale + m_offset.x(),
                   worldPoint.y() * m_scale + m_offset.y());
}

QPointF PlotWidget::screenToWorld(const QPointF &screenPoint) const
{
    qDebug() << "screenToWorld called.";
    return QPointF((screenPoint.x() - m_offset.x()) / m_scale,
                   (screenPoint.y() - m_offset.y()) / m_scale);
}

//QRectF PlotWidget::screenToWorld(const QRectF &screenRect) const
//{
//    QPointF topLeft = screenToWorld(screenRect.topLeft());
//    QPointF bottomRight = screenToWorld(screenRect.bottomRight());
//    return QRectF(topLeft, bottomRight);
//}

// 返回图件尺寸（X坐标和Y坐标范围构成的QRect）
void PlotWidget::updateDataRect()
{
    qDebug() << "updateDataRect called.";
    if (!m_dataPointData || m_dataPointData->points.isEmpty())
        return;

    double minX = m_dataPointData->points[0].coordinate.x();
    double maxX = minX;
    double minY = m_dataPointData->points[0].coordinate.y();
    double maxY = minY;

    for (const DataPoint &point : m_dataPointData->points) {
        if (!point.isVisible) continue;

        minX = qMin(minX, point.coordinate.x());
        maxX = qMax(maxX, point.coordinate.x());
        minY = qMin(minY, point.coordinate.y());
        maxY = qMax(maxY, point.coordinate.y());
    }

    m_dataRect = QRectF(minX, minY, maxX - minX, maxY - minY);
}

///drawLines的功能已并入drawPoints中
//void PlotWidget::drawLines(QPainter &painter)
//{
//    qDebug() << "drawLines called.";
//    if (!m_datFileData)
//        return;

//    QVector<LineSegment> segments = getQualitySegmentedLines();

//    for (const LineSegment &segment : segments) {
//        if (segment.pointIndices.size() < 2) continue;

//        // 选择颜色
//        painter.setPen(QPen(segment.hasNormalAlt ? m_normalAltColor : m_abnormalAltColor, 2));

//        // 绘制线段
//        for (int i = 1; i < segment.pointIndices.size(); ++i) {
//            int idx1 = segment.pointIndices[i-1];
//            int idx2 = segment.pointIndices[i];

//            if (idx1 >= m_datFileData->points.size() || idx2 >= m_datFileData->points.size())
//                continue;

//            const DataPoint &p1 = m_datFileData->points[idx1];
//            const DataPoint &p2 = m_datFileData->points[idx2];

//            if (!p1.isVisible || !p2.isVisible) continue;

//            QPointF screen1 = worldToScreen(p1.coordinate);
//            QPointF screen2 = worldToScreen(p2.coordinate);

//            painter.drawLine(screen1, screen2);
//        }
//    }
//}

void PlotWidget::drawPoints(QPainter &painter)
{
    qDebug() << "drawPoints called.";
    if (!m_dataPointData)
        return;

//    for (const DataPoint &point : m_dataPointData->points) {
//        if (!point.isVisible) continue;

//        QPointF screenPos = worldToScreen(point.coordinate);

//        // 根据质量选择颜色
//        QColor color = ((point.alt >= m_dataPointData->lowAltThreshold)
//                        && (point.alt <= m_dataPointData->highAltThreshold))
//                        ?m_normalAltColor : m_abnormalAltColor;

//        painter.setPen(QPen(color, 1));
//        painter.setBrush(QBrush(color));

//        // 绘制小圆点
//        painter.drawEllipse(screenPos, 3, 3);
//    }

    DataPoint lastDataPoint = m_dataPointData->points[0];
    QPointF lastScreenPos = worldToScreen(lastDataPoint.coordinate);
    for (int i = 1; i < m_dataPointData->points.size(); ++i) {
        const DataPoint& currentDataPoint = m_dataPointData->points[i];
        QPointF currentScreenPos = worldToScreen(currentDataPoint.coordinate);
        if (lastDataPoint.isVisible) {
            QColor color = lastDataPoint.isNormalAlt ?m_normalAltColor : m_abnormalAltColor;
            painter.setPen(QPen(color, 1));
            painter.setBrush(QBrush(color));
            painter.drawEllipse(lastScreenPos, 3, 3);
        }
        if (currentDataPoint.isVisible) {
            QColor color = currentDataPoint.isNormalAlt ?m_normalAltColor : m_abnormalAltColor;
            painter.setPen(QPen(color, 1));
            painter.setBrush(QBrush(color));
            painter.drawEllipse(currentScreenPos, 3, 3);
        }
        if (lastDataPoint.isVisible && currentDataPoint.isVisible
                && (lastDataPoint.fn + 20 > currentDataPoint.fn)
                && (lastDataPoint.lineId == currentDataPoint.lineId)) {
            painter.setPen(QPen(m_lineSegmentColor, 1));
            painter.setBrush(QBrush(m_lineSegmentColor));
            painter.drawLine(currentScreenPos, lastScreenPos);
        }
        lastDataPoint = currentDataPoint;
        lastScreenPos = currentScreenPos;
    }
}

//void PlotWidget::drawSelectionRegions(QPainter &painter)
//{
//    painter.setPen(QPen(Qt::yellow, 2, Qt::DashLine));
//    painter.setBrush(QBrush(m_selectionColor));

//    for (const QRectF &region : m_selectionRegions) {
//        QPointF topLeft = worldToScreen(region.topLeft());
//        QPointF bottomRight = worldToScreen(region.bottomRight());
//        QRectF screenRect(topLeft, bottomRight);

//        painter.drawRect(screenRect);
//    }
//}

void PlotWidget::drawGrid(QPainter &painter)
{
    qDebug() << "drawGrid called.";
    // 简单的网格绘制
    painter.setPen(QPen(Qt::lightGray, 1));

    QRect viewRect = rect();

    // 垂直线
    for (int x = 0; x < viewRect.width(); x += 50) {
        painter.drawLine(x, 0, x, viewRect.height());
    }

    // 水平线
    for (int y = 0; y < viewRect.height(); y += 50) {
        painter.drawLine(0, y, viewRect.width(), y);
    }
}

QVector<LineSegment> PlotWidget::getQualitySegmentedLines() const
{
    qDebug() << "getQualitySegmentedLines called.";
    QVector<LineSegment> segments;

    if (!m_dataPointData)
        return segments;

    // 遍历每条线
    for (auto it = m_dataPointData->lineMap.begin(); it != m_dataPointData->lineMap.end(); ++it) {
        const QString &lineNumber = it.key();
        const QVector<int> &pointIndices = it.value();

        if (pointIndices.size() < 2) continue;

        LineSegment currentSegment;
        currentSegment.lineId = lineNumber;

        for (int idx : pointIndices) {
            if (idx >= m_dataPointData->points.size()) continue;

            const DataPoint &point = m_dataPointData->points[idx];
            if (!point.isVisible) continue;

            bool isHighQuality = (point.alt >= m_dataPointData->lowAltThreshold)
                                && (point.alt <= m_dataPointData->highAltThreshold);

            // 如果质量状态改变，开始新的段
            if (!currentSegment.pointIndices.isEmpty() &&
                currentSegment.hasNormalAlt != isHighQuality) {

                if (currentSegment.pointIndices.size() >= 2) {
                    segments.append(currentSegment);
                }

                currentSegment = LineSegment();
                currentSegment.lineId = lineNumber;
                currentSegment.hasNormalAlt = isHighQuality;
            } else if (currentSegment.pointIndices.isEmpty()) {
                currentSegment.hasNormalAlt = isHighQuality;
            }

            currentSegment.pointIndices.append(idx);
        }

        // 添加最后一个段
        if (currentSegment.pointIndices.size() >= 2) {
            segments.append(currentSegment);
        }
    }

    return segments;
}

bool PlotWidget::isPointSelected(const QPointF &point) const
{
    qDebug() << "isPointSelected called.";
//    for (const QRectF &region : m_selectionRegions) {
//        if (region.contains(point)) {
//            return true;
//        }
//    }
//    return false;
    if (m_selectionPolygon_screen.containsPoint(point, Qt::OddEvenFill)){
        return true;
    }
    return false;
}

void PlotWidget::updateSelectionMode()
{
    qDebug() << "updateSelectionMode called.";
    QApplication *app = qobject_cast<QApplication*>(QApplication::instance());
    Qt::KeyboardModifiers modifiers = app->keyboardModifiers();

    if (modifiers & Qt::ShiftModifier) {
        m_selectionMode = Add;
    } else if (modifiers & Qt::ControlModifier) {
        m_selectionMode = Subtract;
    } else {
        m_selectionMode = Normal;
    }
}

//PolygonSelectionWidget::PolygonSelectionWidget(QWidget *parent)
//    : QWidget(parent), m_isSelecting(false)
//{
//    setMouseTracking(true); // 启用鼠标跟踪
//}

//void PolygonSelectionWidget::clearSelection()
//{
//    m_points.clear();
//    m_selectionPolygon.clear();
////    m_isSelecting = false;
//    update();
//}

//QPolygonF PolygonSelectionWidget::getSelection() const
//{
//    return m_selectionPolygon;
//}

//void PolygonSelectionWidget::clearSelectedArea()
//{
//    if (m_selectionPolygon.isEmpty())
//        return;

//    // 这里实现清除选区内的内容的逻辑
//    // 例如：重绘背景或删除选区内的图形元素
//    update();
//}

//void PolygonSelectionWidget::paintEvent(QPaintEvent *event)
//{
//    Q_UNUSED(event);

//    QPainter painter(this);
//    painter.setRenderHint(QPainter::Antialiasing);

//    // 绘制你原有的内容
//    // ...


//    if (!m_points.isEmpty()) {
//        painter.setPen(QPen(Qt::blue, 2, Qt::DotLine));
//        painter.setBrush(QBrush(QColor(100, 100, 255, 50))); // 半透明填充

//        // 绘制已确定的线段
//        for (int i = 1; i < m_points.size(); ++i) {
//            painter.drawLine(m_points[i-1], m_points[i]);
//        }

//        // 绘制当前正在绘制的线段
//        if (m_isSelecting && !m_points.isEmpty()) {
//            painter.drawLine(m_points.last(), m_currentPoint);
//        }

//        // 如果有多边形完成，绘制填充
//        if (!m_selectionPolygon.isEmpty()) {
//            painter.drawPolygon(m_selectionPolygon);
//        }
//    }
//}

//void PolygonSelectionWidget::mousePressEvent(QMouseEvent *event)
//{
//    if (event->button() == Qt::LeftButton) {
//        if (!m_isSelecting) {
//            // 开始新的选区
//            m_points.clear();
//            m_selectionPolygon.clear();
//            m_isSelecting = true;
//        }

//        m_points.append(event->pos());
//        update();
//    }
//    else if (event->button() == Qt::RightButton && m_isSelecting) {
//        // 右键取消当前选区
//        clearSelection();
//    }
//}

//void PolygonSelectionWidget::mouseMoveEvent(QMouseEvent *event)
//{
//    if (m_isSelecting) {
//        m_currentPoint = event->pos();
//        update();
//    }
//}

//void PolygonSelectionWidget::mouseDoubleClickEvent(QMouseEvent *event)
//{
//    if (event->button() == Qt::LeftButton && m_isSelecting && m_points.size() >= 3) {
//        // 双击完成多边形
//        m_selectionPolygon = QPolygonF(m_points);
//        m_isSelecting = false;
//        emit selectionCompleted(m_selectionPolygon);
//        update();
//    }
//}
