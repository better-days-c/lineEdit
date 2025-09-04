#include "previewdialog.h"
#include <QMessageBox>
#include <QLabel>
#include <QtWidgets>
#include <QDebug>
PreviewDialog::PreviewDialog(const QString &fileName, QWidget *parent)
    : QDialog(parent), m_fileName(fileName), m_skipLines(0)
{
    setWindowTitle("文件配置");
    setModal(true);
    resize(800, 600);

//    m_textEdit = new PreviewTextEdit(this);
//    m_textEdit->setReadOnly(true);
//    m_textEdit->setLineWrapMode(QPlainTextEdit::NoWrap); // 不自动换行，保持原格式

//    m_spinBox = new QSpinBox(this);
//    m_spinBox->setRange(0, 100); // 假设最多100行头数据
//    m_spinBox->setValue(0);
//    m_spinBox->setToolTip("选择要跳过的文件开头行数 (不包括表头)");

//    QLabel *spinLabel = new QLabel("跳过前 N 行:", this);

//    m_okButton = new QPushButton("确定", this);
//    m_cancelButton = new QPushButton("取消", this);

//    QHBoxLayout *spinLayout = new QHBoxLayout();
//    spinLayout->addWidget(spinLabel);
//    spinLayout->addWidget(m_spinBox);
//    spinLayout->addStretch(); // 填充空白

//    QHBoxLayout *buttonLayout = new QHBoxLayout();
//    buttonLayout->addStretch();
//    buttonLayout->addWidget(m_okButton);
//    buttonLayout->addWidget(m_cancelButton);

//    QVBoxLayout *mainLayout = new QVBoxLayout(this);
//    mainLayout->addWidget(m_textEdit);
//    mainLayout->addLayout(spinLayout);
//    mainLayout->addLayout(buttonLayout);

//    connect(m_okButton, &QPushButton::clicked, this, &PreviewDialog::acceptDialog);
//    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
//    connect(m_spinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
//            m_textEdit, &PreviewTextEdit::shadowExcludedLines);

//    loadPreviewContent();
    loadFilePreview();
    setupUI();
    updateDataPreview();
}

void PreviewDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 创建步骤切换组件
    m_stackedWidget = new QStackedWidget(this);

    setupStep1();
    setupStep2();

    m_stackedWidget->addWidget(m_step1Widget);
    m_stackedWidget->addWidget(m_step2Widget);
    m_stackedWidget->setCurrentIndex(0);

    mainLayout->addWidget(m_stackedWidget);

    // 通用取消按钮
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    m_cancelButton = new QPushButton("取消", this);
    connect(m_cancelButton, &QPushButton::clicked, this, &PreviewDialog::onCancelClicked);
    buttonLayout->addWidget(m_cancelButton);

    mainLayout->addLayout(buttonLayout);
}

void PreviewDialog::setupStep1()
{
    m_step1Widget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(m_step1Widget);

    // 标题
    QLabel *titleLabel = new QLabel("步骤 1: 设置要跳过的行数");
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(titleFont.pointSize() + 2);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    layout->addWidget(titleLabel);

    // 跳过行数控制
    QHBoxLayout *skipLayout = new QHBoxLayout();
    skipLayout->addWidget(new QLabel("跳过前"));

    m_skipLinesSpinBox = new QSpinBox();
    m_skipLinesSpinBox->setRange(0, 100);
    m_skipLinesSpinBox->setValue(m_skipLines);
    connect(m_skipLinesSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &PreviewDialog::onSkipLinesChanged);
    skipLayout->addWidget(m_skipLinesSpinBox);

    skipLayout->addWidget(new QLabel("行"));
    skipLayout->addStretch();

    layout->addLayout(skipLayout);

    // 文件预览
    QLabel *previewLabel = new QLabel("文件预览（前100行）:");
    layout->addWidget(previewLabel);

    m_previewEdit = new PreviewTextEdit(this);
    m_previewEdit->setReadOnly(true);
    m_previewEdit->setLineWrapMode(QPlainTextEdit::NoWrap);
    m_previewEdit->setFont(QFont("Consolas", 9));
    layout->addWidget(m_previewEdit);

    // 状态信息
    m_previewStatusLabel = new QLabel();
    layout->addWidget(m_previewStatusLabel);

    // 下一步按钮
    QHBoxLayout *step1ButtonLayout = new QHBoxLayout();
    step1ButtonLayout->addStretch();

    m_nextButton = new QPushButton("下一步 →");
    m_nextButton->setDefault(true);
    step1ButtonLayout->addWidget(m_nextButton);
    connect(m_nextButton, &QPushButton::clicked, this, &PreviewDialog::onNextStepClicked);
    connect(m_skipLinesSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            m_previewEdit, &PreviewTextEdit::shadowExcludedLines);
    layout->addLayout(step1ButtonLayout);
}

void PreviewDialog::setupStep2()
{
    m_step2Widget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(m_step2Widget);

    // 标题
    QLabel *titleLabel = new QLabel("步骤 2: 列映射设置");
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(titleFont.pointSize() + 2);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    layout->addWidget(titleLabel);

    // 创建分割器
    m_step2Splitter = new QSplitter(Qt::Horizontal);

    // 左侧：列映射控制
    m_mappingControlGroup = new QGroupBox("列映射设置");
    m_mappingControlGroup->setMaximumWidth(300);
    m_mappingControlGroup->setMinimumWidth(250);

    QGridLayout *mappingLayout = new QGridLayout(m_mappingControlGroup);

    // 创建下拉框
    m_lineNumberCombo = new QComboBox();
    m_pointNumberCombo = new QComboBox();
    m_xCoordinateCombo = new QComboBox();
    m_yCoordinateCombo = new QComboBox();
    m_offsetCombo = new QComboBox();

    // 连接信号
    connect(m_lineNumberCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &PreviewDialog::onColumnMappingChanged);
    connect(m_pointNumberCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &PreviewDialog::onColumnMappingChanged);
    connect(m_xCoordinateCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &PreviewDialog::onColumnMappingChanged);
    connect(m_yCoordinateCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &PreviewDialog::onColumnMappingChanged);
    connect(m_offsetCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &PreviewDialog::onColumnMappingChanged);

    // 布局
    mappingLayout->addWidget(new QLabel("线号列:"), 0, 0);
    mappingLayout->addWidget(m_lineNumberCombo, 0, 1);

    mappingLayout->addWidget(new QLabel("点号列:"), 1, 0);
    mappingLayout->addWidget(m_pointNumberCombo, 1, 1);

    mappingLayout->addWidget(new QLabel("X坐标列:"), 2, 0);
    mappingLayout->addWidget(m_xCoordinateCombo, 2, 1);

    mappingLayout->addWidget(new QLabel("Y坐标列:"), 3, 0);
    mappingLayout->addWidget(m_yCoordinateCombo, 3, 1);

    mappingLayout->addWidget(new QLabel("数据质量列:"), 4, 0);
    mappingLayout->addWidget(m_offsetCombo, 4, 1);

    mappingLayout->setRowStretch(5, 1);

    // 右侧：数据预览表格
    m_previewTable = new QTableWidget();
    m_previewTable->setAlternatingRowColors(true);
    m_previewTable->setSelectionBehavior(QAbstractItemView::SelectColumns);
    m_previewTable->horizontalHeader()->setStretchLastSection(false);
    m_previewTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);

    // 组装分割器
    m_step2Splitter->addWidget(m_mappingControlGroup);
    m_step2Splitter->addWidget(m_previewTable);
    m_step2Splitter->setStretchFactor(0, 0);
    m_step2Splitter->setStretchFactor(1, 1);

    layout->addWidget(m_step2Splitter);

    // 按钮布局
    QHBoxLayout *step2ButtonLayout = new QHBoxLayout();

    m_previousButton = new QPushButton("← 上一步");
    connect(m_previousButton, &QPushButton::clicked, this, &PreviewDialog::onPreviousStepClicked);
    step2ButtonLayout->addWidget(m_previousButton);

    step2ButtonLayout->addStretch();

    m_acceptButton = new QPushButton("确定");
    m_acceptButton->setDefault(true);
    connect(m_acceptButton, &QPushButton::clicked, this, &PreviewDialog::onAcceptClicked);
    step2ButtonLayout->addWidget(m_acceptButton);

    layout->addLayout(step2ButtonLayout);
}

void PreviewDialog::loadFilePreview()
{
    QFile file(m_fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "错误", "无法打开文件: " + m_fileName);
        return;
    }
    QTextStream in(&file);
    in.setCodec("UTF-8");
    m_originalData.clear();

    int lineCount = 0;
    QString lastLine;
    while (!in.atEnd() && lineCount < PREVIEW_ROWS) {
        lastLine = in.readLine();
        m_originalData.append(lastLine);
        m_previewText += lastLine + "\n";
        lineCount++;
    }

    int colSize = lastLine.split(' ', QString::SkipEmptyParts).size();
    int headerLines = 0;
    for (const QString &line : m_originalData)
    {
        if (line.split(' ', QString::SkipEmptyParts).size() == colSize)
        {
            break;
        }
        else headerLines ++;
    }
//    m_skipLinesSpinBox->setValue(headerLines);
    m_skipLines = headerLines;
    file.close();
}

void PreviewDialog::updateDataPreview()
{
    m_skipLines = m_skipLinesSpinBox->value();

    // 更新过滤后的数据
    m_filteredData.clear();
    for (int i = m_skipLines; i < m_originalData.size(); ++i) {
        m_filteredData.append(m_originalData[i]);
    }
    // 更新预览文本
    QString previewText;

    m_previewEdit->setPlainText(m_previewText);

    m_previewEdit->shadowExcludedLines(m_skipLines);


    // 解析表头
    parseHeaders();
}

void PreviewDialog::parseHeaders()
{
    m_headers.clear();

    if (m_filteredData.isEmpty()) {
        return;
    }

    // 第一行是表头
    QString headerLine = m_filteredData.first();
    m_headers = headerLine.split(QRegExp("\\s+"), QString::SkipEmptyParts);

    // 更新下拉框选项
    QStringList comboOptions;
    comboOptions << "（请选择）";

    for (int i = 0; i < m_headers.size(); ++i) {
        comboOptions << QString("第%1列: %2").arg(i + 1).arg(m_headers[i]);
    }

    m_lineNumberCombo->clear();
    m_lineNumberCombo->addItems(comboOptions);

    m_pointNumberCombo->clear();
    m_pointNumberCombo->addItems(comboOptions);

    m_xCoordinateCombo->clear();
    m_xCoordinateCombo->addItems(comboOptions);

    m_yCoordinateCombo->clear();
    m_yCoordinateCombo->addItems(comboOptions);

    m_offsetCombo->clear();
    m_offsetCombo->addItems(comboOptions);

    // 尝试智能匹配
    for (int i = 0; i < m_headers.size(); ++i) {
        QString header = m_headers[i].toLower();

        if (header == "line")   m_lineNumberCombo->setCurrentIndex(i + 1);
        else if (header == "fn")    m_pointNumberCombo->setCurrentIndex(i + 1);
        else if (header == "x")     m_xCoordinateCombo->setCurrentIndex(i + 1);
        else if (header == "y")     m_yCoordinateCombo->setCurrentIndex(i + 1);
        else if (header == "offset")    m_offsetCombo->setCurrentIndex(i + 1);
    }
}

void PreviewDialog::updateColumnMappingPreview()
{
    if (m_filteredData.isEmpty() || m_headers.isEmpty()) {
        return;
    }

    // 设置表格
    m_previewTable->setColumnCount(m_headers.size());
    m_previewTable->setHorizontalHeaderLabels(m_headers);

    // 设置预览行数
    int previewRows = qMin(PREVIEW_TABLE_ROWS, m_filteredData.size() - 1); // 减1是因为第一行是表头
    m_previewTable->setRowCount(previewRows);

    // 填充数据（跳过表头行）
    for (int row = 0; row < previewRows && (row + 1) < m_filteredData.size(); ++row) {
        QString dataLine = m_filteredData[row + 1]; // +1跳过表头
        QStringList columns = dataLine.split(QRegExp("\\s+"), QString::SkipEmptyParts);

        for (int col = 0; col < qMin(columns.size(), m_headers.size()); ++col) {
            QTableWidgetItem *item = new QTableWidgetItem(columns[col]);
            m_previewTable->setItem(row, col, item);
        }
    }

    // 高亮选中的列
    highlightSelectedColumns();
}

void PreviewDialog::highlightSelectedColumns()
{
    // 重置所有列的背景色
    for (int col = 0; col < m_previewTable->columnCount(); ++col) {
        for (int row = 0; row < m_previewTable->rowCount(); ++row) {
            QTableWidgetItem *item = m_previewTable->item(row, col);
            if (item) {
                item->setBackground(QBrush());
            }
        }
        // 重置表头背景
        QTableWidgetItem *headerItem = m_previewTable->horizontalHeaderItem(col);
        if (headerItem) {
            headerItem->setBackground(QBrush(Qt::white));
        }
    }

    // 高亮选中的列
    QList<QPair<int, QColor>> selectedColumns;

    if (m_columnMapping.lineIdColumn >= 0) {
        selectedColumns.append({m_columnMapping.lineIdColumn, QColor(255, 200, 200)});
    }
    if (m_columnMapping.fnColumn >= 0) {
        selectedColumns.append({m_columnMapping.fnColumn, QColor(200, 255, 200)});
    }
    if (m_columnMapping.xCoordinateColumn >= 0) {
        selectedColumns.append({m_columnMapping.xCoordinateColumn, QColor(200, 200, 255)});
    }
    if (m_columnMapping.yCoordinateColumn >= 0) {
        selectedColumns.append({m_columnMapping.yCoordinateColumn, QColor(255, 255, 200)});
    }
    if (m_columnMapping.offsetColumn >= 0) {
        selectedColumns.append({m_columnMapping.offsetColumn, QColor(255, 200, 255)});
    }

    for (const auto &colInfo : selectedColumns) {
        int col = colInfo.first;
        QColor color = colInfo.second;

        // 高亮表头
        QTableWidgetItem *headerItem = m_previewTable->horizontalHeaderItem(col);
        if (headerItem) {
            headerItem->setBackground(QBrush(color));
        }

        // 高亮数据行
        for (int row = 0; row < m_previewTable->rowCount(); ++row) {
            QTableWidgetItem *item = m_previewTable->item(row, col);
            if (item) {
                item->setBackground(QBrush(color));
            }
        }
    }
}

bool PreviewDialog::validateColumnMapping()
{
    // 检查是否所有必需的列都已选择
    if (!m_columnMapping.isValid()) {
        QMessageBox::warning(this, "配置不完整", "请为所有必需的列选择对应的表头。");
        return false;
    }

    // 检查是否有重复选择
    QSet<int> selectedColumns;
    selectedColumns.insert(m_columnMapping.lineIdColumn);
    selectedColumns.insert(m_columnMapping.fnColumn);
    selectedColumns.insert(m_columnMapping.xCoordinateColumn);
    selectedColumns.insert(m_columnMapping.yCoordinateColumn);
    selectedColumns.insert(m_columnMapping.offsetColumn);

    if (selectedColumns.size() != 5) {
        QMessageBox::warning(this, "列选择冲突", "不能将多个功能映射到同一列。");
        return false;
    }

    return true;
}

void PreviewDialog::onSkipLinesChanged(int lines)
{
    updateDataPreview();
}

void PreviewDialog::onNextStepClicked()
{
    if (m_filteredData.isEmpty()) {
        QMessageBox::warning(this, "数据为空", "过滤后没有可用数据，请调整跳过行数。");
        return;
    }

    m_stackedWidget->setCurrentIndex(1);
    updateColumnMappingPreview();
}

void PreviewDialog::onPreviousStepClicked()
{
    m_stackedWidget->setCurrentIndex(0);
}

void PreviewDialog::onColumnMappingChanged()
{
    // 更新列映射
    m_columnMapping.lineIdColumn = m_lineNumberCombo->currentIndex() - 1;
    m_columnMapping.fnColumn = m_pointNumberCombo->currentIndex() - 1;
    m_columnMapping.xCoordinateColumn = m_xCoordinateCombo->currentIndex() - 1;
    m_columnMapping.yCoordinateColumn = m_yCoordinateCombo->currentIndex() - 1;
    m_columnMapping.offsetColumn = m_offsetCombo->currentIndex() - 1;

    // 更新预览高亮
    highlightSelectedColumns();
}

void PreviewDialog::onAcceptClicked()
{
    if (validateColumnMapping()) {
        accept();
    }
}

void PreviewDialog::onCancelClicked()
{
    reject();
}

int PreviewDialog::getHeaderLinesToSkip() const
{
    return m_headerLinesToSkip;
}

void PreviewDialog::acceptDialog()
{
    m_headerLinesToSkip = m_skipLinesSpinBox->value();
    accept();
}

void PreviewDialog::loadPreviewContent()
{
//    QFile file(m_fileName);
//    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
//        m_textEdit->setPlainText("无法打开文件进行预览: " + file.errorString());
//        return;
//    }

//    QTextStream in(&file);
//    in.setCodec("UTF-8");
//    QString previewText;
//    int lineCount = 0;
//    const int maxPreviewLines = 100; // 只预览文件的前100行

//    QString lastLine;
//    while (!in.atEnd() && lineCount < maxPreviewLines) {
//        lastLine = in.readLine();
//        previewText += lastLine + "\n";
//        lineCount++;
//    }

//    QStringList parsed = lastLine.split(' ', QString::SkipEmptyParts);
//    int colSize = parsed.size();

//    int headerLineSize = 0;
//    QStringList lines = previewText.split('\n');
//    for (const QString &line : lines)
//    {
//        if (line.split(' ', QString::SkipEmptyParts).size() == colSize)
//        {
//            break;
//        }
//        else headerLineSize++;
//    }
//    m_skipLinesSpinBox->setValue(headerLineSize);

//    file.close();
//    m_textEdit->setPlainText(previewText);
//    m_textEdit->shadowExcludedLines(headerLineSize);
}

//PreviewTextEdit的构造函数
PreviewTextEdit::PreviewTextEdit(QWidget *parent) : QPlainTextEdit(parent)
{
    lineNumberArea = new LineNumberArea(this);

    connect(this, &PreviewTextEdit::updateRequest,
            this, &PreviewTextEdit::updateLineNumberArea);

    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void PreviewTextEdit::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), Qt::lightGray);

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int)blockBoundingRect(block).height();

    while (block.isValid() &&top <= event->rect().bottom())
    {
        if (block.isVisible() && bottom >= event->rect().top())
        {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(Qt::darkGray);
            painter.drawText(0, top, lineNumberArea->width(), fontMetrics().height(),
                             Qt::AlignRight, number);
        }
        block = block.next();
        top = bottom;
        bottom = top + (int)blockBoundingGeometry(block).height();
        ++blockNumber;
    }
}

int PreviewTextEdit::lineNumberAreaWidth()
{
    int digits = 3;
    int space = 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
    return space;
}

void PreviewTextEdit::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void PreviewTextEdit::updateLineNumberArea(const QRect &rect)
{
    lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void PreviewTextEdit::shadowExcludedLines(int n) {
    QList<QTextEdit::ExtraSelection> selections;

    QTextDocument *doc = document();

    for (int i = 0; i < n; ++i) {
        QTextBlock block = doc->findBlockByNumber(i);
        if (block.isValid()) {
            QTextEdit::ExtraSelection selection;

            QColor lineColor = QColor(Qt::lightGray);
            selection.format.setProperty(QTextFormat::FullWidthSelection, true);
            selection.format.setBackground(lineColor);

            selection.cursor = QTextCursor(block);
            selection.cursor.clearSelection();

            selections.append(selection);
        }
    }

    setExtraSelections(selections);
}

