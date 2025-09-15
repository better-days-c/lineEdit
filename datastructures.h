#ifndef DATAMODEL_H
#define DATAMODEL_H
#pragma once
#include <QString>
#include <QObject>
#include <QVector>
#include <QTableView>

// 数据点结构
struct DataPoint {
    QString lineId;     //线号
    int fn;             //点号
    QPointF coordinate; //xy坐标
    double alt;      //雷达高度
    bool isVisible;     //是否在视图中可见

    DataPoint(): fn(0), coordinate(0,0), alt(0.0), isVisible(true) {}

    DataPoint(const QString& line, int point, double x, double y, double alt)
        : lineId(line), fn(point), coordinate(x, y), alt(alt), isVisible(true) {}

    QJsonObject toJson() const;
    static DataPoint fromJson(const QJsonObject& json);
};

// 设计线结构
struct DesignLine {
    QString line;
    double x1;
    double y1;
    double x2;
    double y2;
    double matchTimes = 0;

    QJsonObject toJson() const;
    static DesignLine fromJson(const QJsonObject& json);
};

//线段结构
struct LineSegment {
    QVector<int> pointIndices;  // 引用原始数据点的索引
    QString lineId;             // 线号
    bool hasNormalAlt;          // 是否包含正常高度点


    LineSegment() : hasNormalAlt(true) {}
};

class DataPointData{
public:
    QVector<DataPoint> points;              // 所有数据点
    QHash<QString, QVector<int>> lineMap;   // 线号到点索引的映射，<线号, [fn1, fn2, ...]>
    QString fileName;                       // 文件名
    double lowAltThreshold;                 // 高度下阈
    double highAltThreshold;

    DataPointData() : lowAltThreshold(80), highAltThreshold(120) {}  //缺省阈值

    void addPoint(const DataPoint& point){
        int index = points.size();
        points.append(point);
        lineMap[point.lineId].append(index);
    }

    QVector<LineSegment> getVisibleLineSegments() const;

    void setThreshold(double lowThreshold, double highThreshold){
        lowAltThreshold = lowThreshold;
        highAltThreshold = highThreshold;
    }

    //获取指定线号的所有点索引
    QVector<int> getPointIndicesForLine(const QString& lineNumber) const{
        return lineMap.value(lineNumber);
    }

    // 删除高度异常点（视图层）
//    void hideByOffset(double threshold);

    //删除选区点（视图层）
    void hideByRegion(const QPolygonF& region, bool invert = false);

    ///需重写！重新生成线号
    void regenerateLineNumbers();

    ///为单个线段匹配线号
    QString matchLineNumber(int fn, const QJsonObject& designObj);  // 输入: 每个线段的第一个点号; 输出: 匹配到的线号

private:
    //检查高度正常
    bool isNormalAlt(double alt) const{
        return (alt >= lowAltThreshold) && (alt <= highAltThreshold);
    }
};

#endif  //DATASRUCTURE_H
