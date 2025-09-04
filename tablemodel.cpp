#include "tablemodel.h"
#include "QMessageBox"

TableModel::TableModel(QObject *parent)
    :QAbstractTableModel(parent), m_headerLinesToSkip(0), m_previewLines(0)
{
}

bool TableModel::setFile(const QString &filePath, int headerLinesToSkip)
{
    beginResetModel(); // 准备重置模型数据

    m_filePath = filePath;
    m_headerLinesToSkip = headerLinesToSkip;
    m_columnHeaders.clear();
    m_data.clear();

    if (!loadDataFromFile()) {
        endResetModel(); // 结束重置
        return false;
    }

    endResetModel(); // 结束重置
    return true;
}

void TableModel::clear()
{
    beginResetModel();
    m_filePath.clear();
    m_headerLinesToSkip = 0;
    m_columnHeaders.clear();
    m_data.clear();
    m_previewLines.clear();
    m_previewRowCount = 0;
    endResetModel();
}

int TableModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_data.count();
}

int TableModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_columnHeaders.count(); // 列数由表头决定
}

QVariant TableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= m_data.size() || index.column() >= m_columnHeaders.size())
        return QVariant();

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        return m_data[index.row()][index.column()];
    }
    return QVariant();
}

QVariant TableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            if (section < m_columnHeaders.size())
                return m_columnHeaders[section];
        } else { // Vertical header (row numbers)
            return section + 1; // Display row number starting from 1
        }
    }
    return QVariant();
}

bool TableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole) {
        if (index.row() < m_data.size() && index.column() < m_columnHeaders.size()) {
            m_data[index.row()][index.column()] = value.toString();
            emit dataChanged(index, index, {role}); // 发射信号通知视图数据已改变
            return true;
        }
    }
    return false;
}

Qt::ItemFlags TableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;
    return QAbstractTableModel::flags(index) | Qt::ItemIsEditable; // 设置项可编辑
}

// 私有辅助函数：从文件中加载数据
bool TableModel::loadDataFromFile()
{
    QFile file(m_filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(nullptr, "错误", "无法打开文件: " + file.errorString());
        return false;
    }

    QTextStream in(&file);
    QString line;

    // 跳过指定的头行
    for (int i = 0; i < m_headerLinesToSkip; ++i) {
        if (!in.atEnd()) {
            in.readLine(); // 读取但不使用
        }
    }

    // 读取表头
    if (!in.atEnd()) {
        line = in.readLine();
        m_columnHeaders = parseLine(line);
        if (m_columnHeaders.isEmpty()) {
            QMessageBox::warning(nullptr, "警告", "文件没有可解析的表头。");
            file.close();
            return false;
        }
    } else {
        QMessageBox::warning(nullptr, "警告", "文件内容太少，无法找到表头。");
        file.close();
        return false;
    }

    // 读取数据行
    while (!in.atEnd()) {
        line = in.readLine();
        QStringList rowData = parseLine(line);
        // 确保每行的数据项数量与表头一致，或至少不为空
        if (!rowData.isEmpty() && rowData.size() == m_columnHeaders.size()) {
            m_data.append(rowData);
        } else if (!rowData.isEmpty() && rowData.size() != m_columnHeaders.size()) {
             qDebug() << "Warning: Row data size mismatch at line:" << line;
             // 可以选择忽略此行，或者根据需求填充/截断
             // 这里选择忽略不匹配的行
        }
    }

    file.close();
    return true;
}

// 私有辅助函数：解析一行字符串
QStringList TableModel::parseLine(const QString &line) const
{
    // 使用 QRegularExpression 替换多个空格为一个空格，然后按空格分割
    // 或者直接使用 QString::split() 配合 Qt::SkipEmptyParts
    return line.split(' ', QString::SkipEmptyParts);
}

bool TableModel::exportToDat(const QString &filePath) const
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(nullptr, "错误", "无法保存文件: " + file.errorString());
        return false;
    }

    QTextStream out(&file);

    // 写入表头
    out << m_columnHeaders.join(" ") << "\n";

    // 写入数据
    for (const QStringList &row : m_data) {
        out << row.join(" ") << "\n";
    }

    file.close();
    return true;
}

bool TableModel::exportToCsv(const QString &filePath) const
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(nullptr, "错误", "无法保存文件: " + file.errorString());
        return false;
    }

    QTextStream out(&file);

    // 写入CSV表头（逗号分隔，处理包含逗号的字段）
    QStringList csvHeaders;
    for (const QString &header : m_columnHeaders) {
        QString mutableHeader = header;
        csvHeaders << "\"" + mutableHeader.replace("\"", "\"\"") + "\""; // 处理包含引号的字段
    }
    out << csvHeaders.join(",") << "\n";

    // 写入CSV数据
    for (const QStringList &row : m_data) {
        QStringList csvRow;
        for (const QString &item : row) {
            QString mutableItem = item;
            csvRow << "\"" + mutableItem.replace("\"", "\"\"") + "\""; // 处理包含引号的字段
        }
        out << csvRow.join(",") << "\n";
    }

    file.close();
    return true;
}
