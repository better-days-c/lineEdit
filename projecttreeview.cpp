#include "projecttreeview.h"
#include <QContextMenuEvent>
#include <QMenu>
#include <QAction>
#include <QHeaderView>
#include <QMessageBox>
#include <QFileInfo>
#include <QtDebug>

// CheckBoxDelegate实现
CheckBoxDelegate::CheckBoxDelegate(QObject *parent) : QItemDelegate(parent) {}

QWidget *CheckBoxDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                                       const QModelIndex &index) const {
    QCheckBox *editor = new QCheckBox(parent);
    connect(editor, &QCheckBox::toggled, this, [this, index](bool checked) {
        emit checkStateChanged(index, checked);
    });
    return editor;
}

void CheckBoxDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
    bool value = index.model()->data(index, Qt::CheckStateRole).toBool();
    QCheckBox *checkBox = static_cast<QCheckBox*>(editor);
    checkBox->setChecked(value);
}

void CheckBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                   const QModelIndex &index) const {
    QCheckBox *checkBox = static_cast<QCheckBox*>(editor);
    model->setData(index, checkBox->isChecked(), Qt::CheckStateRole);
}

void CheckBoxDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                                          const QModelIndex &index) const {
    editor->setGeometry(option.rect);
}

// ProjectTreeView实现
ProjectTreeView::ProjectTreeView(QWidget *parent) : QTreeView(parent),
    m_treeModel(new QStandardItemModel(this)),
    m_projectModel(nullptr),
    m_checkBoxDelegate(new CheckBoxDelegate(this)) {
    setupTreeModel();

    // 设置代理
    setItemDelegateForColumn(0, m_checkBoxDelegate);

    // 连接信号槽
    connect(m_checkBoxDelegate, &CheckBoxDelegate::checkStateChanged,
            this, [this](const QModelIndex& index, bool checked) {
        if (isDesignLineItem(index)) {
            int designLineIndex = getDesignLineIndex(index);
            emit designLineVisibilityChanged(designLineIndex, checked);
        }
    });

    // 配置树视图
    setHeaderHidden(true);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setContextMenuPolicy(Qt::CustomContextMenu);
//    connect(this, &QTreeView::customContextMenuRequested,
//            this, &ProjectTreeView::contextMenuEvent);
}

void ProjectTreeView::setProjectModel(ProjectModel *model) {
    m_projectModel = model;
    if (m_projectModel) {
        connect(m_projectModel, &ProjectModel::designLinesChanged,
                this, &ProjectTreeView::refreshTree);
        connect(m_projectModel, &ProjectModel::batchesChanged,
                this, &ProjectTreeView::refreshTree);
        refreshTree();
        expandAll();
    }
}

void ProjectTreeView::setupTreeModel() {
    m_treeModel->clear();
    m_treeModel->setColumnCount(1);
}

void ProjectTreeView::refreshTree() {
    if (!m_projectModel) return;

    setupTreeModel();

    // 根节点
    QStandardItem* rootItem = new QStandardItem(m_projectModel->getProjectName());
    rootItem->setData(RootItem, Qt::UserRole);
    m_treeModel->setItem(0, 0, rootItem);

    // 设计线根节点
    QStandardItem* designLinesRoot = new QStandardItem("设计线");
    designLinesRoot->setData(DesignLinesRootItem, Qt::UserRole);
    rootItem->appendRow(designLinesRoot);

    // 测试线根节点
    QStandardItem* testLinesRoot = new QStandardItem("测线");
    testLinesRoot->setData(TestLinesRootItem, Qt::UserRole);
    rootItem->appendRow(testLinesRoot);

    // 填充设计线
    populateDesignLines();

    // 填充测试线
    populateTestLines();

    // 展开所有节点
    expandAll();

    setModel(m_treeModel);
}

void ProjectTreeView::populateDesignLines() {
    if (!m_projectModel) return;

    QStandardItem* designLinesRoot = m_treeModel->item(0, 0)->child(0, 0);
    if (!designLinesRoot) return;

    const auto& designLines = m_projectModel->getDesignLines();
    for (int i = 0; i < designLines.size(); ++i) {
        const auto& designLine = designLines[i];
        QString filePath = designLine.filePath;
        QFileInfo fileInfo(filePath);
        QString fileName = fileInfo.fileName();
        QStandardItem* item = new QStandardItem(fileName);
        item->setData(DesignLineFileItem, Qt::UserRole);
        item->setData(i, Qt::UserRole + 1); // 存储设计线索引
        item->setCheckable(true);
        item->setCheckState(designLine.visible ? Qt::Checked : Qt::Unchecked);
        designLinesRoot->appendRow(item);
    }
}

void ProjectTreeView::populateTestLines() {
    if (!m_projectModel) return;

    QStandardItem* testLinesRoot = m_treeModel->item(0, 0)->child(1, 0);
    if (!testLinesRoot) return;

    const auto& batches = m_projectModel->getBatches();
    for (int i = 0; i < batches.size(); ++i) {
        const auto& batch = batches[i];
        QStandardItem* batchItem = new QStandardItem(batch.batchName);
        batchItem->setData(BatchItem, Qt::UserRole);
        batchItem->setData(i, Qt::UserRole + 1); // 存储批次索引
        testLinesRoot->appendRow(batchItem);

        // 添加测试线文件
        for (int j = 0; j < batch.filePaths.size(); ++j) {
            const auto& filePath = batch.filePaths[j];
            QFileInfo fileInfo(filePath);
            QString fileName = fileInfo.fileName();
            QStandardItem* fileItem = new QStandardItem(fileName);
            fileItem->setData(TestLineFileItem, Qt::UserRole);
            fileItem->setData(i, Qt::UserRole + 1); // 批次索引
            fileItem->setData(j, Qt::UserRole + 2); // 文件索引
            batchItem->appendRow(fileItem);
        }
    }
}

ProjectTreeView::TreeItemType ProjectTreeView::getItemType(const QModelIndex &index) const {
    if (!index.isValid()) return RootItem;
    return static_cast<TreeItemType>(index.data(Qt::UserRole).toInt());
}

QModelIndex ProjectTreeView::getSelectedBatchIndex() const {
    QModelIndexList selection = selectedIndexes();
    if (selection.isEmpty()) return QModelIndex();

    QModelIndex currentIndex = selection.first();
    if (getItemType(currentIndex) == BatchItem) {
        return currentIndex;
    } else if (getItemType(currentIndex) == TestLineFileItem) {
        return currentIndex.parent();
    }

    return QModelIndex();
}

bool ProjectTreeView::isDesignLineItem(const QModelIndex &index) const {
    return getItemType(index) == DesignLineFileItem;
}

bool ProjectTreeView::isTestLineFileItem(const QModelIndex &index) const {
    return getItemType(index) == TestLineFileItem;
}

bool ProjectTreeView::isBatchItem(const QModelIndex &index) const {
    return getItemType(index) == BatchItem;
}

int ProjectTreeView::getDesignLineIndex(const QModelIndex &index) const {
    if (isDesignLineItem(index)) {
        return index.data(Qt::UserRole + 1).toInt();
    }
    return -1;
}

int ProjectTreeView::getBatchIndex(const QModelIndex &index) const {
    if (isBatchItem(index)) {
        return index.data(Qt::UserRole + 1).toInt();
    } else if (isTestLineFileItem(index)) {
        return index.data(Qt::UserRole + 1).toInt();
    }
    return -1;
}

int ProjectTreeView::getTestLineFileIndex(const QModelIndex &index) const {
    if (isTestLineFileItem(index)) {
        return index.data(Qt::UserRole + 2).toInt();
    }
    return -1;
}

void ProjectTreeView::mouseDoubleClickEvent(QMouseEvent *event) {
    QModelIndex index = indexAt(event->pos());
    if (isBatchItem(index) && index.model()) {
        int batchIndex = getBatchIndex(index);
        if (m_projectModel->getBatches()[batchIndex].size){
            emit batchDoubleClicked(batchIndex);
        }
    }
    QTreeView::mouseDoubleClickEvent(event);
}

void ProjectTreeView::contextMenuEvent(QContextMenuEvent *event) {
    QModelIndex index = indexAt(event->pos());
    if (!index.isValid()) return;

    QMenu menu(this);

    // 添加架次动作（仅在测试线根节点上）
    if (getItemType(index) == TestLinesRootItem) {
        QAction* addBatchAction = new QAction("添加架次", this);
        connect(addBatchAction, &QAction::triggered, this, &ProjectTreeView::addBatchRequested);
        menu.addAction(addBatchAction);
    }

    // 删除动作（对可删除的项目）
    TreeItemType itemType = getItemType(index);
    if (itemType == DesignLineFileItem || itemType == BatchItem || itemType == TestLineFileItem) {
        QAction* deleteAction = new QAction("删除", this);
        connect(deleteAction, &QAction::triggered, this, &ProjectTreeView::deleteItemRequested);
        menu.addAction(deleteAction);
    }

    if (!menu.actions().isEmpty()) {
        menu.exec(event->globalPos());
    }
}
