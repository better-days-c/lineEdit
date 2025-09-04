#ifndef PREVIEWDIALOG_H
#define PREVIEWDIALOG_H

#include <QDialog>
#include <QPlainTextEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFile>
#include <QTextStream>
#include <QPaintEvent>
#include <QTableView>
#include <QStackedWidget>
#include <QStackedWidget>
#include <QStringList>
#include <QLabel>
#include <QSplitter>
#include <QGroupBox>
#include <QComboBox>
#include <QTableWidget>

class PreviewTextEdit;
class LineNumberArea;

struct ColumnMapping{
    int lineIdColumn = -1;
    int fnColumn = -1;
    int xCoordinateColumn = -1;
    int yCoordinateColumn = -1;
    int offsetColumn = -1;

    bool isValid() const{
        return lineIdColumn >= 0 && fnColumn >= 0 &&
                xCoordinateColumn >= 0 && yCoordinateColumn >= 0 &&
                offsetColumn >= 0;
    }
};

class PreviewDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PreviewDialog(const QString &filePath, QWidget *parent = nullptr);   
    int getHeaderLinesToSkip() const;
    ColumnMapping getColumnMapping() const { return m_columnMapping; }
    QStringList getHeaders() const { return m_headers; }
    int getSkipLines() const { return m_skipLines; }
    QStringList getFilteredData() const { return m_filteredData;}

private slots:
    void acceptDialog();
    void onSkipLinesChanged(int lines);
    void onNextStepClicked();
    void onPreviousStepClicked();
    void onColumnMappingChanged();
    void onAcceptClicked();
    void onCancelClicked();

private:
    QString m_filePath;
    PreviewTextEdit *m_textEdit;
    QSpinBox *m_spinBox;
    QPushButton *m_okButton;
    QPushButton *m_cancelButton;
    int m_headerLinesToSkip;

//    QTableView *m_previewTable = nullptr;
    QLineEdit * m_skipLinesEdit = nullptr;

    void loadPreviewContent();

    void setupUI();
    void setupStep1();
    void setupStep2();
    void loadFilePreview();
    void updateDataPreview();
    void updateColumnMappingPreview();
    void parseHeaders();
    bool validateColumnMapping();
    void highlightSelectedColumns();

    QString m_fileName;
    QStringList m_originalData;
    QStringList m_filteredData;
    QString m_previewText;
    QStringList m_headers;
    int m_skipLines;
    ColumnMapping m_columnMapping;

    // UI组件
    QStackedWidget *m_stackedWidget;

    // 第一步：跳过行数设置
    QWidget *m_step1Widget;
    PreviewTextEdit *m_previewEdit;
    QSpinBox *m_skipLinesSpinBox;
    QLabel *m_previewStatusLabel;
    QPushButton *m_nextButton;

    // 第二步：列映射设置
    QWidget *m_step2Widget;
    QSplitter *m_step2Splitter;

    // 列映射控制区域
    QGroupBox *m_mappingControlGroup;
    QComboBox *m_lineNumberCombo;
    QComboBox *m_pointNumberCombo;
    QComboBox *m_xCoordinateCombo;
    QComboBox *m_yCoordinateCombo;
    QComboBox *m_offsetCombo;

    // 数据预览表格
    QTableWidget *m_previewTable;
    QPushButton *m_previousButton;
    QPushButton *m_acceptButton;

    // 通用按钮
//    QPushButton *m_cancelButton;

    static const int PREVIEW_ROWS = 100;  // 预览行数
    static const int PREVIEW_TABLE_ROWS = 20;  // 表格预览行数
};

class PreviewTextEdit : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit PreviewTextEdit(QWidget *parent = nullptr);

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();
    void shadowExcludedLines(int);

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void updateLineNumberArea(const QRect &);

private:
    LineNumberArea *lineNumberArea;
};

class LineNumberArea : public QWidget
{
public:
    LineNumberArea(PreviewTextEdit *textEdit) : QWidget(textEdit){
        previewTextEdit = textEdit;
    }

    QSize sizeHint() const override{
        return QSize(previewTextEdit->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event) override{
        previewTextEdit->lineNumberAreaPaintEvent(event);
    }

private:
    PreviewTextEdit *previewTextEdit;
};

#endif // PREVIEWDIALOG_H
