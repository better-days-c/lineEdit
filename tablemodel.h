#ifndef TABLEMODEL_H
#define TABLEMODEL_H

#include <QAbstractTableModel>
#include <QStringList>
#include <QFile>
#include <QTextStream>
#include <QDebug>

class TableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit TableModel(QObject *parent = nullptr);

    // 设置文件路径和跳过的行数
    bool setFile(const QString &filePath, int headerLinesToSkip);
    void clear(); // 清除模型数据

    // QAbstractTableModel 必须实现的虚函数
    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;

    // 可编辑表格需要的虚函数
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) Q_DECL_OVERRIDE;
    Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;

    // 导出数据
    bool exportToDat(const QString &filePath) const;
    bool exportToCsv(const QString &filePath) const;

private:
    QString m_filePath;
    int m_headerLinesToSkip;
    QStringList m_columnHeaders;
    QList<QStringList> m_data; // 存储表格数据

    // 用于预览的临时数据，仅在预览阶段使用
    QStringList m_previewLines;
    int m_previewRowCount;

    // 辅助函数：根据文件路径读取数据到 m_data
    bool loadDataFromFile();

    // 辅助函数：从一行字符串中解析数据
    QStringList parseLine(const QString &line) const;
};

#endif // TABLEMODEL_H
