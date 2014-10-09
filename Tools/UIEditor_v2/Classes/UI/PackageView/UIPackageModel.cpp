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
#include "Utils/QtDavaConvertion.h"
#include "UIPackageModelNode.h"
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
    , root(NULL)
{
    root = new UIPackageModelRootNode(_package, _package->GetName(), UIPackageModelRootNode::MODE_CONTROLS_AND_IMPORTED_PACKAGED);
    
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
    SafeRelease(root);
    delete undoStack;
}

QModelIndex UIPackageModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    if (!parent.isValid())
        return createIndex(row, column, root->Get(row));

    UIPackageModelNode *node = static_cast<UIPackageModelNode*>(parent.internalPointer());
    return createIndex(row, column, node->Get(row));
}

QModelIndex UIPackageModel::parent(const QModelIndex &child) const
{
    if (!child.isValid())
        return QModelIndex();

    UIPackageModelNode *node = static_cast<UIPackageModelNode*>(child.internalPointer());
    UIPackageModelNode *parent = node->GetParent();
    if (parent == NULL || parent == root)
        return QModelIndex();
    
    if (parent->GetParent())
        return createIndex(parent->GetParent()->GetIndex(parent), 0, parent);
    else
        return createIndex(0, 0, parent);
}

int UIPackageModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return root ? root->GetCount() : 0;
    
    return static_cast<UIPackageModelNode*>(parent.internalPointer())->GetCount();
}

int UIPackageModel::columnCount(const QModelIndex &/*parent*/) const
{
    return 1;
}

QVariant UIPackageModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    UIPackageModelNode *node = static_cast<UIPackageModelNode*>(index.internalPointer());
    
    //UIEditorComponent *editorComponent = dynamic_cast<UIEditorComponent*>(control->GetCustomData());
//    DVASSERT(editorComponent != NULL);
//    bool createdFromPrototype = editorComponent != NULL && editorComponent->IsClonedFromPrototype();
    
    switch(role)
    {
    case Qt::DisplayRole:
        {
            return StringToQString(node->GetName());
        }
        break;
    case Qt::DecorationRole:
            return node->GetControl() != NULL ? QIcon(IconHelper::GetIconPathForUIControl(node->GetControl())) : QVariant();
    case Qt::CheckStateRole:
            if (node->GetControl())
                return node->GetControl()->GetVisibleForUIEditor() ? Qt::Checked : Qt::Unchecked;
            else
                return QVariant();
//    case Qt::ToolTipRole:
//        return QString(control->GetControlClassName().c_str());
//    case Qt::EditRole:
//        return QString(control->GetName().c_str());
        case Qt::TextColorRole:
            return node->IsCloned() ? Qt::blue : Qt::black;
            
        case Qt::BackgroundRole:
            return node->IsHeader() ? Qt::lightGray : Qt::white;
            
        case Qt::FontRole:
        {
            QFont myFont;
            if (node->IsInstancedFromPrototype() || node->IsHeader())
                myFont.setBold(true);
            if (!node->IsEditable())
                myFont.setItalic(true);

            return myFont;
        }
            
    default:
        break;
    }

    return QVariant();
}

bool UIPackageModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;
    
    UIPackageModelNode *node = static_cast<UIPackageModelNode*>(index.internalPointer());
    
    if (role == Qt::CheckStateRole)
    {
        if (node->GetControl())
            node->GetControl()->SetVisibleForUIEditor(value.toBool());
        return true;
    }
//    else if (role == Qt::EditRole)
//    {
//        control->SetName(value.toString().toStdString());
//        return true;
//    }
    
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
    flags |= /*Qt::ItemIsEditable | */Qt::ItemIsUserCheckable;
    
//    const UIControl *control = static_cast<UIControl*>(index.internalPointer());
//    
//    if (IsPackageRootControl(control))
//        flags = flags | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
//    else if(IsPackageContentControl(control))
//        flags = flags | Qt::ItemIsDropEnabled;
    
    flags |= Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
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

//bool UIPackageModel::IsPackageRootControl(const UIControl *control) const
//{
//    uint32 count = package->GetControlsCount();
//    for (uint32 index = 0; index < count; ++index)
//    {
//        if (package->GetControl(index) == control)
//            return true;
//    }
//    
//    return false;
//}
//
//bool UIPackageModel::IsPackageContentControl(const UIControl *control) const
//{
//    const UIControl *rootControl = control;
//    while (rootControl->GetParent())
//    {
//        rootControl = rootControl->GetParent();
//    }
//    return IsPackageRootControl(rootControl);
//}

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

