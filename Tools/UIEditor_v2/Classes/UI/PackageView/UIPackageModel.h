//
//  UIPackageModel.h
//  UIEditor
//
//  Created by Dmitry Belsky on 10.9.14.
//
//

#ifndef __UI_EDITOR_UI_PACKAGE_MODEL_H__
#define __UI_EDITOR_UI_PACKAGE_MODEL_H__

#include <QAbstractItemModel>
#include <QMimeData>
#include <QStringList>
#include <QUndoStack>
#include <QUndoCommand>
#include "DAVAEngine.h"

class UIPackageModelNode;

class UIPackageMimeData: public QMimeData
{
    Q_OBJECT
public:
    UIPackageMimeData();
    ~UIPackageMimeData();
    
    void SetIndex(const QModelIndex &_index ){ index = _index;}
    const QModelIndex &GetIndex() const{ return index;}
    
    virtual bool hasFormat(const QString &mimetype) const override;
    virtual QStringList formats() const override;
    
protected:
    virtual QVariant retrieveData(const QString &mimetype, QVariant::Type preferredType) const override;
    
private:
    QPersistentModelIndex index;
};

class UIPackageModel : public QAbstractItemModel
{
    Q_OBJECT
    
public:
    UIPackageModel(DAVA::UIPackage *package, QObject *parent = 0);
    virtual ~UIPackageModel();
    
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    virtual QModelIndex parent(const QModelIndex &child) const override;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const  override;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const  override;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    
    virtual Qt::DropActions supportedDropActions() const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual QStringList mimeTypes() const;
    virtual QMimeData *mimeData(const QModelIndexList &indexes) const;
    
//    virtual bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());
//    virtual bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());
    
    virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);
    
    void InsertControl(DAVA::UIControl *control, const QModelIndex &parent, const QModelIndex &insertBelowIndex);
    void RemoveControl(DAVA::UIControl *control);
    
    void InsertControlToContent(DAVA::UIControl *control, DAVA::UIControl *parent, DAVA::UIControl *insertBelowControl);
    void RemoveControlFromContent(DAVA::UIControl *control);
    
    void InsertControlToPackage(DAVA::UIControl *control, int index);
    void RemoveControlFromPackage(DAVA::UIControl *control);
    
    QAction *UndoAction() const { return undoAction; }
    QAction *RedoAction() const { return redoAction; }
    
    friend class MoveItemModelCommand;
    
    void MoveItem(const QModelIndex &srcItem, int dstRow, const QModelIndex &dstParent);
    void CopyItem(const QModelIndex &srcItem, int dstRow, const QModelIndex &dstParent);
    void RemoveItem(const QModelIndex &srcItem);
    
//private:
//    bool IsPackageRootControl(const DAVA::UIControl *control) const;
//    bool IsPackageContentControl(const DAVA::UIControl *control) const;
private:
    UIPackageModelNode *root;
    
    QUndoStack *undoStack;
    QAction *undoAction;
    QAction *redoAction;
};

class BasePackageModelCommand: public QUndoCommand
{
public:
    BasePackageModelCommand(UIPackageModel *_package, const QString &text, QUndoCommand * parent = 0)
    : QUndoCommand(text, parent)
    , package(_package)
    {}
    
    virtual ~BasePackageModelCommand()
    {
    }
    
    UIPackageModel *GetModel(){return package;}
    
private:
    UIPackageModel *package;
};

class MoveItemModelCommand: public BasePackageModelCommand
{
public:
    MoveItemModelCommand(UIPackageModel *_package, const QModelIndex &srcIndex, int dstRow, const QModelIndex &dstParent, QUndoCommand * parent = 0);
    ~MoveItemModelCommand();
    
    virtual void undo();
    virtual void redo();
private:
    int srcRow;
    int dstRow;
    QPersistentModelIndex srcParent;
    QPersistentModelIndex dstParent;
};

class CopyItemModelCommand: public BasePackageModelCommand
{
public:
    CopyItemModelCommand(UIPackageModel *_package, const QModelIndex &srcIndex, int dstRow, const QModelIndex &dstParent, QUndoCommand * parent = 0);
    ~CopyItemModelCommand();
    
    virtual void undo();
    virtual void redo();
private:
    int srcRow;
    int dstRow;
    QPersistentModelIndex srcParent;
    QPersistentModelIndex dstParent;
};


#endif // __UI_EDITOR_UI_PACKAGE_MODEL_H__
