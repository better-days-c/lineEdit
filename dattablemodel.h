#ifndef DATTABLEMODEL_H
#define DATTABLEMODEL_H

#include <QAbstractTableModel>
#include <QStringList>
#include <QSet>
#include "datastructures.h"
#include "previewdialog.h"

class DatTableModel : public QAbstractTableModel{
    Q_OBJECT

public:
    //列定义
    enum Column{
        LineId = 0,
        FN,
        X_Coordinate,
        Y_Coordinate,
        Offset,
        ColumnCount
    };

    explicit DatTableModel(QObject *parent = nullptr);

    // QAbstractTableModel interface
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // 设置数据
    void setDatFileData(DatFileData *data);

    // 获取原始数据索引（考虑到隐藏的行）
    int getOriginalIndex(int viewRow) const;

    // 刷新可见行
    void refreshVisibleRows();

    // 获取列的可见性
    bool isColumnVisible(Column col) const;

    // 设置列的可见性
    void setColumnVisible(Column col, bool visible);

    // 获取可见的列
    QVector<Column> getVisibleColumns() const;

    // 设置列映射信息
    void setColumnMapping(const ColumnMapping &mapping);

    // 获取所有可用的列（包括扩展列）
    QStringList getAllAvailableColumns() const;

    // 动态添加列
    void addCustomColumn(const QString &columnName);

signals:
    void dataChanged();

private:
    DatFileData *m_datFileData;
    QVector<int> m_visibleRows;     // 可见行在原始数据中的索引
    QSet<Column> m_visibleColumns;  // 可见的列
    QStringList m_headers;
    ColumnMapping m_columnMapping;
    QStringList m_allHeaders;  // 包含所有可能的列头
    QHash<QString, int> m_customColumnMap;  // 自定义列映射

    void updateVisibleRows();
    void initHeaders();
};



#endif // DATTABLEMODEL_H
