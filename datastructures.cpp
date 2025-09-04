#include "datastructures.h"
#include <QDebug>

QVector<LineSegment> DatFileData::getVisibleLineSegments() const
{
    QVector<LineSegment> segments;

    // 遍历每条线
    for (auto it = lineMap.begin(); it != lineMap.end(); ++it) {
        const QString &lineNumber = it.key();
        const QVector<int> &pointIndices = it.value();

        if (pointIndices.size() < 2) continue;

        // 获取可见的点
        QVector<int> visibleIndices;
        for (int idx : pointIndices) {
            if (idx < points.size() && points[idx].isVisible) {
                visibleIndices.append(idx);
            }
        }

        if (visibleIndices.size() < 2) continue;

        // 根据质量分段
        LineSegment currentSegment;
        currentSegment.lineId = lineNumber;

        for (int i = 0; i < visibleIndices.size(); ++i) {
            int idx = visibleIndices[i];
            const DataPoint &point = points[idx];

            bool isHighQuality = (point.offset >= offsetThreshold);

            // 如果质量状态改变或者是第一个点，开始新的段
            if (currentSegment.pointIndices.isEmpty()) {
                currentSegment.hasLowOffset = isHighQuality;
                currentSegment.pointIndices.append(idx);
            } else if (currentSegment.hasLowOffset != isHighQuality) {
                // 质量状态改变，结束当前段并开始新段
                if (currentSegment.pointIndices.size() >= 2) {
                    segments.append(currentSegment);
                }

                // 开始新段，但要包含前一个点以保持连续性
                currentSegment = LineSegment();
                currentSegment.lineId = lineNumber;
                currentSegment.hasLowOffset = isHighQuality;
                if (i > 0) {
                    currentSegment.pointIndices.append(visibleIndices[i-1]);
                }
                currentSegment.pointIndices.append(idx);
            } else {
                // 质量状态相同，继续当前段
                currentSegment.pointIndices.append(idx);
            }
        }

        // 添加最后一个段
        if (currentSegment.pointIndices.size() >= 2) {
            segments.append(currentSegment);
        }
    }

    return segments;
}

void DatFileData::hideByOffset(double threshold)
{
    for (DataPoint &point : points) {
        if (point.offset < threshold) {
            point.isVisible = false;
        }
    }
}

void DatFileData::hideByRegion(const QPolygonF &region, bool invert)
{
    for (DataPoint &point : points) {
        bool inRegion = region.containsPoint(point.coordinate, Qt::OddEvenFill);
        if (invert) {
            // 反选：选区外的点被隐藏
            if (!inRegion) {
                point.isVisible = false;
            }
        } else {
            // 正选：选区内的点被隐藏
            if (inRegion) {
                point.isVisible = false;
            }
        }
    }
}

void DatFileData::regenerateLineNumbers()
{
    // 清空原有的线映射
    lineMap.clear();

    // 重新构建线映射，只包含可见的点
    QHash<QString, QVector<int>> tempLineMap;

    for (int i = 0; i < points.size(); ++i) {
        if (points[i].isVisible) {
            tempLineMap[points[i].lineId].append(i);
        }
    }

    // 对于每条线，检查是否需要分割
    for (auto it = tempLineMap.begin(); it != tempLineMap.end(); ++it) {
        const QString &originalLineNumber = it.key();
        const QVector<int> &pointIndices = it.value();

        if (pointIndices.size() < 2) {
            continue;
        }

        // 检查点号是否连续，如果不连续则分割线
        QVector<QVector<int>> subLines;
        QVector<int> currentSubLine;

        for (int i = 0; i < pointIndices.size(); ++i) {
            int currentIdx = pointIndices[i];

            if (currentSubLine.isEmpty()) {
                currentSubLine.append(currentIdx);
            } else {
                int lastIdx = currentSubLine.last();
                int currentPointNum = points[currentIdx].fn;
                int lastPointNum = points[lastIdx].fn;

                // 如果点号不连续（相差超过1），则开始新的子线
                if (currentPointNum - lastPointNum > 1) {
                    if (currentSubLine.size() >= 2) {
                        subLines.append(currentSubLine);
                    }
                    currentSubLine.clear();
                }
                currentSubLine.append(currentIdx);
            }
        }

        // 添加最后一个子线
        if (currentSubLine.size() >= 2) {
            subLines.append(currentSubLine);
        }

        // 更新线号和映射
        if (subLines.size() == 1)  {
            // 没有分割，保持原线号
            lineMap[originalLineNumber] = subLines[0];
        } else {
            // 分割了，生成新的线号
            for (int subIdx = 0; subIdx < subLines.size(); ++subIdx) {
                QString newLineNumber = QString("%1%2").arg(originalLineNumber).arg(subIdx + 1);

                // 更新点的线号
                for (int pointIdx : subLines[subIdx]) {
                    points[pointIdx].lineId = newLineNumber;
                }

                lineMap[newLineNumber] = subLines[subIdx];
            }
        }
    }
}
