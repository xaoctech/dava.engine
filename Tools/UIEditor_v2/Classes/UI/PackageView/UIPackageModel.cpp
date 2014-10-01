//
//  UIPackageModel.cpp
//  UIEditor
//
//  Created by Dmitry Belsky on 10.9.14.
//
//

#include "UIPackageModel.h"

#include "DAVAEngine.h"
#include "IconHelper.h"
#include "UIControls/UIEditorComponent.h"
#include <qicon.h>
#include <QAction>

using namespace DAVA;

UIPackageMimeData::UIPackageMimeData()
{
}

UIPackageMimeData::~UIPackageMimeData()
{
}

bool UIPackageMimeData::hasFormat(const QString &mimetype) const
{
    if (mimetype == "application/packageModel")
        return true;
    return QMimeData::hasFormat(mimetype);
}

QStringList UIPackageMimeData::formats() const
{
    QStringList types;
    types << "application/packageModel";
    return types;
}

QVariant UIPackageMimeData::retrieveData(const QString &mimetype, QVariant::Type preferredType) const
{
    if (mimetype == "application/packageModel")
        return QVariant(QVariant::UserType);
    
    return QMimeData::retrieveData(mimetype, preferredType);
}

UIPackageModel::UIPackageModel(DAVA::UIPackage *_package, QObject *parent)
    : QAbstractItemModel(parent)
    , package(SafeRetain(_package))
{
    
    undoStack = new QUndoStack(this);
    undoAction = undoStack->createUndoAction(this, tr("&Undo"));
    undoAction->setShortcuts(QKeySequence::Undo);
    undoAction->setIcon(QIcon(":/Icons/118.png"));
    
    redoAction = undoStack->createRedoAction(this, tr("&Redo"));
    redoAction->setShortcuts(QKeySequence::Redo);
    redoAction->setIcon(QIcon(":/Icons/117.png"));
}

UIPackageModel::~UIPackageModel()
{
    SafeRelease(package);
    delete undoStack;
}

QModelIndex UIPackageModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    if (!parent.isValid())
    {
        return createIndex(row, column, package->GetControl(row));
    }

    UIControl *control = static_cast<UIControl*>(parent.internalPointer());
    
    if (row >= (int)control->GetChildren().size())
        return QModelIndex();

    List<UIControl*>::const_iterator iter = control->GetChildren().begin();
    std::advance(iter, row);
    return createIndex(row, column, (*iter));
}

int UIPackageModel::GetControlRow(UIControl *control) const
{
    UIControl *parent = control->GetParent();
    if (parent)
    {
        List<UIControl*>::const_iterator iter = parent->GetChildren().begin();
        List<UIControl*>::const_iterator end = parent->GetChildren().end();

        int index = 0;
        for (; iter != end; ++iter, ++index)
        {
            if (*iter == control)
                return index;
        }
    }
    else
    {
        uint32 count = package->GetControlsCount();
        for (uint32 index = 0; index < count; ++index)
        {
            if (package->GetControl(index) == control)
                return index;
        }
    }

    DVASSERT(false);
    return 0;
}

QModelIndex UIPackageModel::parent(const QModelIndex &child) const
{
    if (!child.isValid())
        return QModelIndex();

    UIControl *control = static_cast<UIControl*>(child.internalPointer());

    uint32 count = package->GetControlsCount();
    for (uint32 index = 0; index < count; ++index)
    {
        if (package->GetControl(index) == control)
            return QModelIndex();
    }

    UIControl *parentControl = control->GetParent();
    if (!parentControl)
    {
        DAVA::Logger::Debug("[UIPackageModel::parent] Control without parent \"%s\"", control->GetName().c_str());
        return QModelIndex();
    }

    return createIndex(GetControlRow(parentControl), 0, parentControl);
}

int UIPackageModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return package->GetControlsCount();

    return static_cast<UIControl*>(parent.internalPointer())->GetChildren().size();
}

int UIPackageModel::columnCount(const QModelIndex &/*parent*/) const
{
    return 1;
}

QVariant UIPackageModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    UIControl *control = static_cast<UIControl*>(index.internalPointer());
    
    UIEditorComponent *editorComponent = dynamic_cast<UIEditorComponent*>(control->GetCustomData());
//    DVASSERT(editorComponent != NULL);
    bool createdFromPrototype = editorComponent != NULL && editorComponent->IsClonedFromPrototype();
    
    switch(role)
    {
    case Qt::DisplayRole:
        {
            QString controlName = control->GetName().c_str();
//            if (controlName.isEmpty())
//                controlName = "<Empty>";

            return controlName;
        }
        break;
    case Qt::DecorationRole:
        return QIcon(IconHelper::GetIconPathForUIControl(control));
    case Qt::CheckStateRole:
        return control->GetVisibleForUIEditor() ? Qt::Checked : Qt::Unchecked;
    case Qt::ToolTipRole:
        return QString(control->GetControlClassName().c_str());
    case Qt::EditRole:
        return QString(control->GetName().c_str());
    case Qt::TextColorRole:
        return createdFromPrototype ? Qt::blue : Qt::black;
            
    default:
        break;
    }

    return QVariant();
}

bool UIPackageModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;
    
    UIControl *control = static_cast<UIControl*>(index.internalPointer());
    
    if (role == Qt::CheckStateRole)
    {
        control->SetVisibleForUIEditor(value.toBool());
        return true;
    }
    else if (role == Qt::EditRole)
    {
        control->SetName(value.toString().toStdString());
        return true;
    }
    
    return false;
}

Qt::DropActions UIPackageModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

Qt::ItemFlags UIPackageModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;
    
    Qt::ItemFlags flags = QAbstractItemModel::flags(index);
    flags |= Qt::ItemIsEditable | Qt::ItemIsUserCheckable;
    
//    const UIControl *control = static_cast<UIControl*>(index.internalPointer());
//    
//    if (IsPackageRootControl(control))
//        flags = flags | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
//    else if(IsPackageContentControl(control))
//        flags = flags | Qt::ItemIsDropEnabled;
    
    flags |= Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
    if (int(flags) != 63)
        DAVA::Logger::Debug("[UIPackageModel::flags] %d", (int)flags);
    return flags;
}

QStringList UIPackageModel::mimeTypes() const
{
    QStringList types;
    types << "application/packageModel";
    return types;
}

QMimeData *UIPackageModel::mimeData(const QModelIndexList &indexes) const
{
    UIPackageMimeData *mimeData = new UIPackageMimeData();
    
    foreach (QModelIndex index, indexes)
    {
        if (index.isValid())
        {
            mimeData->SetIndex(index);
        }
    }

    return mimeData;
}

bool UIPackageModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    if (action == Qt::IgnoreAction)
        return true;
    
    if (!data->hasFormat("application/packageModel"))
        return false;
    
    const UIPackageMimeData *controlMimeData = dynamic_cast<const UIPackageMimeData*>(data);
    if (!controlMimeData)
        return false;
    
    if (column > 0)
        return false;

    int rowIndex;
    
    if (row != -1)
    {
        rowIndex = row;
    }
    else if (parent.isValid())
    {
        rowIndex = rowCount(parent);
    }
    else
    {
        rowIndex = rowCount(QModelIndex());
    }
    
    int rows = 1;
    
    QModelIndex srcIndex = controlMimeData->GetIndex();
    QModelIndex srcParent = srcIndex.parent();
    QModelIndex dstParent = parent;
    int dstRow = rowIndex;
    
    if (action == Qt::CopyAction)
    {
        QUndoCommand *changeCommand = new CopyItemModelCommand(this, srcIndex, dstRow, dstParent);
        undoStack->push(changeCommand);
    }
    else if (action == Qt::MoveAction)
    {
        int srcRow = srcIndex.row();
        
        if (srcIndex == dstParent)
        {
            return false;
        }
        
        if (srcParent == dstParent &&
            (dstRow == srcRow || dstRow == srcRow + 1))
        {
            return false;
        }
        
        QUndoCommand *changeCommand = new MoveItemModelCommand(this, srcIndex, dstRow, dstParent);
        undoStack->push(changeCommand);
    }
    
    return true;
}

void UIPackageModel::MoveItem(const QModelIndex &srcItem, int dstRow, const QModelIndex &dstParent)
{
    beginMoveRows(srcItem.parent(), srcItem.row(), srcItem.row(), dstParent, dstRow);
    {
        QModelIndex insertBelowIndex = index(dstRow, 0, dstParent);
    
        ScopedPtr<UIControl> control(SafeRetain(static_cast<UIControl*>(srcItem.internalPointer())));
        RemoveControl(control);
        InsertControl(control, dstParent, insertBelowIndex);
    }
    endMoveRows();
}

void UIPackageModel::CopyItem(const QModelIndex &srcItem, int dstRow, const QModelIndex &dstParent)
{
    beginInsertRows(dstParent, dstRow, dstRow);
    {
        QModelIndex insertBelowIndex = index(dstRow, 0, dstParent);
    
        ScopedPtr<UIControl> control(static_cast<UIControl*>(srcItem.internalPointer())->Clone());
        InsertControl(control, dstParent, insertBelowIndex);
    }
    endInsertRows();
}

void UIPackageModel::RemoveItem(const QModelIndex &srcItem)
{
    beginRemoveRows(srcItem.parent(), srcItem.row(), srcItem.row());
    {
        ScopedPtr<UIControl> control(SafeRetain(static_cast<UIControl*>(srcItem.internalPointer())));
        RemoveControl(control);
    }
    UIControl * removedControl = static_cast<UIControl*>(srcItem.internalPointer());
    endRemoveRows();
}

bool UIPackageModel::IsPackageRootControl(const UIControl *control) const
{
    uint32 count = package->GetControlsCount();
    for (uint32 index = 0; index < count; ++index)
    {
        if (package->GetControl(index) == control)
            return true;
    }
    
    return false;
}

bool UIPackageModel::IsPackageContentControl(const UIControl *control) const
{
    const UIControl *rootControl = control;
    while (rootControl->GetParent())
    {
        rootControl = rootControl->GetParent();
    }
    return IsPackageRootControl(rootControl);
}

void UIPackageModel::InsertControl(DAVA::UIControl *control, const QModelIndex &parent, const QModelIndex &insertBelowIndex)
{
    if (parent.isValid())
    {
        DAVA::UIControl *parentControl = static_cast<UIControl*>(parent.internalPointer());
        DAVA::UIControl *insertBelowControl = static_cast<UIControl*>(insertBelowIndex.internalPointer());
        InsertControlToContent(control, parentControl, insertBelowControl);
    }
    else
    {
        InsertControlToPackage(control, insertBelowIndex.row());
    }
}

void UIPackageModel::RemoveControl(DAVA::UIControl *control)
{
    if (control->GetParent())
    {
        RemoveControlFromContent(control);
    }
    else
    {
        RemoveControlFromPackage(control);
    }
}

void UIPackageModel::InsertControlToContent(DAVA::UIControl *control, DAVA::UIControl *parent, DAVA::UIControl *insertBelowControl)
{
    if (!insertBelowControl)
    {
        parent->AddControl(control);
    }
    else
    {
        parent->InsertChildBelow(control, insertBelowControl);
    }
}

void UIPackageModel::RemoveControlFromContent(DAVA::UIControl *control)
{
    control->RemoveFromParent();
}

void UIPackageModel::InsertControlToPackage(DAVA::UIControl *control, int index)
{
    
}

void UIPackageModel::RemoveControlFromPackage(DAVA::UIControl *control)
{
    
}

MoveItemModelCommand::MoveItemModelCommand(UIPackageModel *_package, const QModelIndex &srcIndex, int _dstRow, const QModelIndex &_dstParent, QUndoCommand *parent)
: BasePackageModelCommand(_package, "Move item", parent)
, srcRow(srcIndex.row())
, dstRow(_dstRow)
, srcParent(srcIndex.parent())
, dstParent(_dstParent)
{
    
}

MoveItemModelCommand::~MoveItemModelCommand()
{
    
}

void MoveItemModelCommand::undo()
{
    int fixedDstRow = dstRow;
    int fixedSrcRow = srcRow;
    if (srcParent == dstParent)
    {
        if (dstRow > srcRow)
        {
            --fixedDstRow;
        }
        else
        {
            ++fixedSrcRow;
        }
            
    }
    QModelIndex dstIndex = GetModel()->index(fixedDstRow, 0, dstParent);
    GetModel()->MoveItem(dstIndex, fixedSrcRow, srcParent);
}

void MoveItemModelCommand::redo()
{
    QModelIndex srcIndex = GetModel()->index(srcRow, 0, srcParent);
    GetModel()->MoveItem(srcIndex, dstRow, dstParent);
}

CopyItemModelCommand::CopyItemModelCommand(UIPackageModel *_package, const QModelIndex &srcIndex, int _dstRow, const QModelIndex &_dstParent, QUndoCommand *parent)
: BasePackageModelCommand(_package, "Move item", parent)
, srcRow(srcIndex.row())
, dstRow(_dstRow)
, srcParent(srcIndex.parent())
, dstParent(_dstParent)
{
    
}

CopyItemModelCommand::~CopyItemModelCommand()
{
    
}

void CopyItemModelCommand::undo()
{
    QModelIndex dstIndex = GetModel()->index(dstRow, 0, dstParent);
    GetModel()->RemoveItem(dstIndex);
}

void CopyItemModelCommand::redo()
{
    QModelIndex srcIndex = GetModel()->index(srcRow, 0, srcParent);
    GetModel()->CopyItem(srcIndex, dstRow, dstParent);
}

