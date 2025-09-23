#include "dattablemodel.h"
#include <QColor>

DatTableModel::DatTableModel(QObject *parent)
    : QAbstractTableModel(parent)
    , m_datFileData(nullptr)
{
    initHeaders();

    // 默认显示重要的列
    m_visibleColumns.insert(LineId);
    m_visibleColumns.insert(FN);
    m_visibleColumns.insert(X_Coordinate);
    m_visibleColumns.insert(Y_Coordinate);
    m_visibleColumns.insert(Alt);
}

void DatTableModel::initHeaders()
{
    m_headers.clear();
    m_headers << "线号" << "点号" << "X坐标" << "Y坐标" << "雷达高度";
}

int DatTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_visibleRows.size();
}

int DatTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_visibleColumns.size();
}

QVariant DatTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || !m_datFileData)
        return QVariant();

    if (index.row() >= m_visibleRows.size())
        return QVariant();

    int originalIndex = m_visibleRows[index.row()];
    if (originalIndex >= m_datFileData->points.size())
        return QVariant();

    const DataPoint &point = m_datFileData->points[originalIndex];

    // 获取实际的列索引
    QVector<Column> visibleCols = getVisibleColumns();
    if (index.column() >= visibleCols.size())
        return QVariant();

    Column actualColumn = visibleCols[index.column()];

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (actualColumn) {
        case LineId:
            return point.lineId;
        case FN:
            return point.fn;
        case X_Coordinate:
            return QString::number(point.coordinate.x(), 'f', 6);
        case Y_Coordinate:
            return QString::number(point.coordinate.y(), 'f', 6);
        case Alt:
            return QString::number(point.alt, 'f', 3);
        default:
            // 处理扩展列
            if (actualColumn >= ColumnCount) {
                // TODO: 从原始数据中获取额外列的值
                return QString("扩展列%1").arg(actualColumn - ColumnCount + 1);
            }
            return QVariant();
        }
    }
    else if (role == Qt::BackgroundRole) {
        // 根据数据质量设置背景色
        if (actualColumn == Alt) {
            if ((point.alt >= m_datFileData->lowAltThreshold) && (point.alt <= m_datFileData->highAltThreshold)) {
                return QColor(200, 255, 200); // 淡绿色表示正常高度
            } else {
                return QColor(255, 200, 200); // 淡红色表示异常高度
            }
        }
        // 根据列映射高亮对应的列
        if (m_columnMapping.lineIdColumn >= 0 && actualColumn == LineId) {
            return QColor(255, 240, 240);
        }
        if (m_columnMapping.xCoordinateColumn >= 0 && actualColumn == X_Coordinate) {
            return QColor(240, 240, 255);
        }
        if (m_columnMapping.yCoordinateColumn >= 0 && actualColumn == Y_Coordinate) {
            return QColor(240, 255, 240);
        }
    }
    return QVariant();
}

QVariant DatTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        QVector<Column> visibleCols = getVisibleColumns();
        if (section < visibleCols.size()) {
            return m_headers[visibleCols[section]];
        }
    }
    return QVariant();
}

void DatTableModel::setBatchData(DataPointData *data)
{
    beginResetModel();
    m_datFileData = data;
    updateVisibleRows();
    endResetModel();
}

int DatTableModel::getOriginalIndex(int viewRow) const
{
    if (viewRow >= 0 && viewRow < m_visibleRows.size()) {
        return m_visibleRows[viewRow];
    }
    return -1;
}

void DatTableModel::refreshVisibleRows()
{
    beginResetModel();
    updateVisibleRows();
    endResetModel();
    emit dataChanged();
}

bool DatTableModel::isColumnVisible(Column col) const
{
    return m_visibleColumns.contains(col);
}

void DatTableModel::setColumnVisible(Column col, bool visible)
{
    if (visible) {
        if (!m_visibleColumns.contains(col)) {
            beginResetModel();
            m_visibleColumns.insert(col);
            endResetModel();
        }
    } else {
        if (m_visibleColumns.contains(col)) {
            beginResetModel();
            m_visibleColumns.remove(col);
            endResetModel();
        }
    }
}

QVector<DatTableModel::Column> DatTableModel::getVisibleColumns() const
{
    QVector<Column> result;
    // 按照预定义顺序返回可见列
    for (int i = 0; i < ColumnCount; ++i) {
        Column col = static_cast<Column>(i);
        if (m_visibleColumns.contains(col)) {
            result.append(col);
        }
    }
    return result;
}

void DatTableModel::updateVisibleRows()
{
    m_visibleRows.clear();

    if (!m_datFileData)
        return;

    for (int i = 0; i < m_datFileData->points.size(); ++i) {
        if (m_datFileData->points[i].isVisible) {
            m_visibleRows.append(i);
        }
    }
}

void DatTableModel::setColumnMapping(const ColumnMapping &mapping)
{
    beginResetModel();
    m_columnMapping = mapping;

    // 更新表头信息
    m_allHeaders.clear();
    m_allHeaders << "线号" << "点号" << "X坐标" << "Y坐标" << "雷达高度";

    // 可以根据需要添加更多列
    for (int i = 5; i < 20; ++i) {  // 假设最多支持20列
        m_allHeaders << QString("列%1").arg(i + 1);
    }

    endResetModel();
}

QStringList DatTableModel::getAllAvailableColumns() const
{
    return m_allHeaders;
}

void DatTableModel::addCustomColumn(const QString &columnName)
{
    if (!m_allHeaders.contains(columnName)) {
        beginResetModel();
        m_allHeaders.append(columnName);
        endResetModel();
    }
}

