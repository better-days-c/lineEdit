#ifndef PROJECTTREEVIEW_H
#define PROJECTTREEVIEW_H

#include <QTreeView>
#include <QStandardItemModel>
#include <QItemDelegate>
#include <QCheckBox>
#include <QMouseEvent>
#include "projectmodel.h"

// 自定义代理，用于在树视图中显示复选框
class CheckBoxDelegate : public QItemDelegate
{
    Q_OBJECT

public:
    explicit CheckBoxDelegate(QObject *parent = nullptr);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                             const QModelIndex &index) const override;

signals:
    void checkStateChanged(const QModelIndex& index, bool checked) const;
};

// 项目树视图类
class ProjectTreeView : public QTreeView
{
    Q_OBJECT

public:
    explicit ProjectTreeView(QWidget *parent = nullptr);

    void setProjectModel(ProjectModel* model);
    QModelIndex getSelectedBatchIndex() const;
    bool isDesignLineItem(const QModelIndex& index) const;
    bool isTestLineFileItem(const QModelIndex& index) const;
    bool isBatchItem(const QModelIndex& index) const;
    int getDesignLineIndex(const QModelIndex& index) const;
    int getBatchIndex(const QModelIndex& index) const;
    int getTestLineFileIndex(const QModelIndex& index) const;

signals:
    void designLineVisibilityChanged(int index, bool visible);
    void addBatchRequested();
    void deleteItemRequested();
    void batchDoubleClicked(int batchIndex);

public slots:
    void refreshTree();

protected:
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    QStandardItemModel* m_treeModel;
    ProjectModel* m_projectModel;
    CheckBoxDelegate* m_checkBoxDelegate;

    // 树节点索引
    enum TreeItemType {
        RootItem,
        DesignLinesRootItem,
        TestLinesRootItem,
        DesignLineFileItem,
        BatchItem,
        TestLineFileItem
    };

    void setupTreeModel();
    void populateDesignLines();
    void populateTestLines();
    TreeItemType getItemType(const QModelIndex& index) const;
};

#endif // PROJECTTREEVIEW_H
