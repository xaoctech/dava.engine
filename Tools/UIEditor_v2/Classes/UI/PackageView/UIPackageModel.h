#ifndef __UI_EDITOR_UI_PACKAGE_MODEL_H__
#define __UI_EDITOR_UI_PACKAGE_MODEL_H__

#include <QAbstractItemModel>
#include <QMimeData>
#include <QStringList>
#include <QUndoStack>
#include "DAVAEngine.h"

class PackageNode;
class ControlNode;
class PackageBaseNode;

class UIPackageModel : public QAbstractItemModel
{
    Q_OBJECT
    
public:
    UIPackageModel(PackageNode *package, QObject *parent = 0);
    virtual ~UIPackageModel();
    
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    virtual QModelIndex parent(const QModelIndex &child) const override;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const  override;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const  override;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    
    virtual Qt::DropActions supportedDragActions() const;
    virtual Qt::DropActions supportedDropActions() const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual QStringList mimeTypes() const;
    virtual QMimeData *mimeData(const QModelIndexList &indexes) const;
    
//    virtual bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());
//    virtual bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());
    
    virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);
    
    void InsertNode(ControlNode *node, const QModelIndex &parent, const QModelIndex &insertBelowIndex);
    void RemoveNode(ControlNode *node);
    
    QAction *UndoAction() const { return undoAction; }
    QAction *RedoAction() const { return redoAction; }
    
    friend class MoveItemModelCommand;
    
    void InsertItem(const QString &name, int dstRow, const QModelIndex &dstParent);
    void MoveItem(const QModelIndex &srcItem, int dstRow, const QModelIndex &dstParent);
    void CopyItem(const QModelIndex &srcItem, int dstRow, const QModelIndex &dstParent);
    void RemoveItem(const QModelIndex &srcItem);
    
private:
    PackageNode *root;
    
    QUndoStack *undoStack;
    QAction *undoAction;
    QAction *redoAction;
};

#endif // __UI_EDITOR_UI_PACKAGE_MODEL_H__
