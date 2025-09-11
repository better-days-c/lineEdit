#ifndef DATAMODEL_H
#define DATAMODEL_H
#pragma once
#include <QString>
#include <QObject>
#include <QVector>
#include <QTableView>

//数据点结构
struct DataPoint {
    QString lineId;     //线号
    int fn;             //点号
    QPointF coordinate; //xy坐标
    double offset;      //偏航
    bool isVisible;     //是否在视图中可见

    DataPoint(): fn(0), coordinate(0,0), offset(0.0), isVisible(true) {}

    DataPoint(const QString& line, int point, double x, double y, double ofst)
        : lineId(line), fn(point), coordinate(x, y), offset(ofst), isVisible(true) {}
};

struct DesignLine {
    QString lineId; //线号
    QPointF startCoordinate; //起始坐标
    QPointF endCoordinate;  //结束坐标
    bool isVisible; //是否在视图中可见

    DesignLine(const QString& line, double startX, double startY, double endX, double endY)
        : lineId(line), startCoordinate(startX, startY), endCoordinate(endX, endY), isVisible(true) {}
};

//线段结构
struct LineSegment {
    QVector<int> pointIndices;  // 引用原始数据点的索引
    QString lineId;             // 线号
    bool hasLowOffset;          // 是否包含低偏航点


    LineSegment() : hasLowOffset(true) {}
};

class DatFileData{
public:
    QVector<DataPoint> points;              // 所有数据点
    QHash<QString, QVector<int>> lineMap;   // 线号到点索引的映射
    QString fileName;                       // 文件名
    double offsetThreshold;                 // 偏航阈值

    DatFileData() : offsetThreshold(60) {}  //缺省阈值

    void addPoint(const DataPoint& point){
        int index = points.size();
        points.append(point);
        lineMap[point.lineId].append(index);
    }

    QVector<LineSegment> getVisibleLineSegments() const;

    void setThreshold(double threshold){
        offsetThreshold = threshold;
    }

    //获取指定线号的所有点索引
    QVector<int> getPointIndicesForLine(const QString& lineNumber) const{
        return lineMap.value(lineNumber);
    }

    // 删除高偏航点（视图层）
    void hideByOffset(double threshold);

    //删除选区点（视图层）
    void hideByRegion(const QPolygonF& region, bool invert = false);

    //重新生成线号
    void regenerateLineNumbers();

private:
    //检查是否低偏航
    bool isLowOffset(double offset) const{
        return offset <= offsetThreshold;
    }
};

#endif  //DATASRUCTURE_H
