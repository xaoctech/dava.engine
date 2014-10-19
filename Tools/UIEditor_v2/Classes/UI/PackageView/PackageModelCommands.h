#ifndef __UI_EDITOR_PACKAGE_MODEL_COMMANDS_H__
#define __UI_EDITOR_PACKAGE_MODEL_COMMANDS_H__

#include <QUndoCommand>
#include <QModelIndex>

class UIPackageModel;

class BasePackageModelCommand: public QUndoCommand
{
public:
    BasePackageModelCommand(UIPackageModel *_packageModel, const QString &text, QUndoCommand * parent = 0)
    : QUndoCommand(text, parent)
    , packageModel(_packageModel)
    {}
    
    virtual ~BasePackageModelCommand()
    {
    }
    
    UIPackageModel *GetModel() {
        return packageModel;
    }
    
private:
    UIPackageModel *packageModel;
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

class InsertControlNodeCommand: public BasePackageModelCommand
{
public:
    InsertControlNodeCommand(UIPackageModel *_package, const QString &controlName, int dstRow, const QModelIndex &dstParent, QUndoCommand * parent = 0);
    ~InsertControlNodeCommand();
    
    virtual void undo();
    virtual void redo();
private:
    int dstRow;
    QPersistentModelIndex dstParent;
    const QString &controlName;
};


#endif // __UI_EDITOR_PACKAGE_MODEL_COMMANDS_H__
