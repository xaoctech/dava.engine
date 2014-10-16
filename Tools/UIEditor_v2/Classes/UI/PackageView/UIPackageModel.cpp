//
//  UIPackageModel.cpp
//  UIEditor
//
//  Created by Dmitry Belsky on 10.9.14.
//
//

#include "UIPackageModel.h"

#include "DAVAEngine.h"
#include "UI/IconHelper.h"
#include "Utils/QtDavaConvertion.h"
#include "UIControls/PackageHierarchy/PackageNode.h"
#include "UIControls/PackageHierarchy/ControlNode.h"
#include "UIControls/PackageHierarchy/PackageControlsNode.h"
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

UIPackageModel::UIPackageModel(PackageNode *_package, QObject *parent)
    : QAbstractItemModel(parent)
    , root(NULL)
{
    root = SafeRetain(_package);
    if (root)
        root->debugDump(0);
    
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

    PackageBaseNode *node = static_cast<PackageBaseNode*>(parent.internalPointer());
    return createIndex(row, column, node->Get(row));
}

QModelIndex UIPackageModel::parent(const QModelIndex &child) const
{
    if (!child.isValid())
        return QModelIndex();

    PackageBaseNode *node = static_cast<PackageBaseNode*>(child.internalPointer());
    PackageBaseNode *parent = node->GetParent();
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
    
    return static_cast<PackageBaseNode*>(parent.internalPointer())->GetCount();
}

int UIPackageModel::columnCount(const QModelIndex &/*parent*/) const
{
    return 1;
}

QVariant UIPackageModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    PackageBaseNode *node = static_cast<PackageBaseNode*>(index.internalPointer());
    
    int prototypeFlag = PackageBaseNode::FLAG_CONTROL_CREATED_FROM_PROTOTYPE | PackageBaseNode::FLAG_CONTROL_CREATED_FROM_PROTOTYPE_CHILD;
    int controlFlag = PackageBaseNode::FLAG_CONTROL_CREATED_FROM_CLASS | PackageBaseNode::FLAG_CONTROL_CREATED_FROM_PROTOTYPE | PackageBaseNode::FLAG_CONTROL_CREATED_FROM_PROTOTYPE_CHILD;
    int flags = node->GetFlags();
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
        case Qt::TextColorRole:
            return (flags & prototypeFlag) != 0 ? Qt::blue : Qt::black;
            
        case Qt::BackgroundRole:
            return (flags & controlFlag) == 0 ? Qt::lightGray : Qt::white;
            
        case Qt::FontRole:
        {
            QFont myFont;
            if ((flags & PackageBaseNode::FLAG_CONTROL_CREATED_FROM_PROTOTYPE) != 0 || (flags & controlFlag) == 0)
                myFont.setBold(true);
            if ((flags & PackageBaseNode::FLAG_READ_ONLY) != 0)
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
    
    PackageBaseNode *node = static_cast<PackageBaseNode*>(index.internalPointer());
    
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

Qt::DropActions UIPackageModel::supportedDragActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

Qt::DropActions UIPackageModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

Qt::ItemFlags UIPackageModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;
    
    Qt::ItemFlags flags = QAbstractItemModel::flags(index) | Qt::ItemIsUserCheckable;
    
    const PackageBaseNode *node = static_cast<PackageBaseNode*>(index.internalPointer());
    if ((node->GetFlags() & PackageBaseNode::FLAGS_CONTROL) != 0)
        flags |= Qt::ItemIsDragEnabled;
    if ((node->GetFlags() & PackageBaseNode::FLAG_READ_ONLY) == 0)
        flags |= Qt::ItemIsDropEnabled;
    
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
        return true;
    }
    else if (action == Qt::MoveAction)
    {
        int srcRow = srcIndex.row();
        
        if (srcIndex == dstParent)
        {
            return false;
        }
        
        if (srcParent == dstParent && (dstRow == srcRow || dstRow == srcRow + 1))
        {
            return false;
        }

        ControlNode *sourceNode = dynamic_cast<ControlNode*>(static_cast<PackageBaseNode*>(index(srcRow, 0, srcParent).internalPointer()));
        if (!sourceNode || (sourceNode->GetFlags() & PackageBaseNode::FLAG_READ_ONLY) != 0)
        {
            return false;
        }

        
        QUndoCommand *changeCommand = new MoveItemModelCommand(this, srcIndex, dstRow, dstParent);
        undoStack->push(changeCommand);
        return true;
    }
    else if (action == Qt::LinkAction)
    {
        DVASSERT(false);
        return true;
    }
    
    return false;
}

void UIPackageModel::MoveItem(const QModelIndex &srcItem, int dstRow, const QModelIndex &dstParent)
{
    ControlNode *sourceNode(dynamic_cast<ControlNode*>(static_cast<PackageBaseNode*>(srcItem.internalPointer())));
    if (sourceNode)
    {
        beginMoveRows(srcItem.parent(), srcItem.row(), srcItem.row(), dstParent, dstRow);
        {
            QModelIndex insertBelowIndex = index(dstRow, 0, dstParent);
            SafeRetain(sourceNode);
            RemoveNode(sourceNode);
            InsertNode(sourceNode, dstParent, insertBelowIndex);
            SafeRelease(sourceNode);
        }
        endMoveRows();
    }
    else
    {
        DVASSERT(false);
    }
}

void UIPackageModel::CopyItem(const QModelIndex &srcItem, int dstRow, const QModelIndex &dstParent)
{
    ControlNode *sourceNode = dynamic_cast<ControlNode*>(static_cast<PackageBaseNode*>(srcItem.internalPointer()));
    if (sourceNode)
    {
        beginInsertRows(dstParent, dstRow, dstRow);
        {
            QModelIndex insertBelowIndex = index(dstRow, 0, dstParent);
            ControlNode* node = new ControlNode(sourceNode, NULL, ControlNode::CREATED_FROM_CLASS);
            InsertNode(node, dstParent, insertBelowIndex);
            SafeRelease(node);
        }
        endInsertRows();
    }
    else
    {
        DVASSERT(false);
    }
}

void UIPackageModel::RemoveItem(const QModelIndex &srcItem)
{
    ControlNode *sourceNode = dynamic_cast<ControlNode*>(static_cast<PackageBaseNode*>(srcItem.internalPointer()));
    if (sourceNode)
    {
        beginRemoveRows(srcItem.parent(), srcItem.row(), srcItem.row());
        RemoveNode(sourceNode);
        endRemoveRows();
    }
    else
    {
        DVASSERT(false);
    }
}

void UIPackageModel::InsertNode(ControlNode *node, const QModelIndex &parent, const QModelIndex &insertBelowIndex)
{
    if (parent.isValid())
    {
        PackageBaseNode *parentNode = static_cast<PackageBaseNode*>(parent.internalPointer());
        if ((parentNode->GetFlags() & PackageBaseNode::FLAG_READ_ONLY) == 0)
        {
            if ((parentNode->GetFlags() & PackageBaseNode::FLAGS_CONTROL) != 0)
            {
                ControlNode *parentControl = dynamic_cast<ControlNode*>(parentNode);
                if (parentControl)
                {
                    ControlNode *belowThis = static_cast<ControlNode*>(insertBelowIndex.internalPointer());
                    if (belowThis)
                        parentControl->InsertBelow(node, belowThis);
                    else
                        parentControl->Add(node);
                }
                else
                {
                    DVASSERT(false);
                }
            }
            else
            {
                PackageControlsNode *parentControls = dynamic_cast<PackageControlsNode*>(parentNode);
                if (parentControls)
                {
                    ControlNode *belowThis = static_cast<ControlNode*>(insertBelowIndex.internalPointer());
                    if (belowThis)
                        parentControls->InsertBelow(node, belowThis);
                    else
                        parentControls->Add(node);
                }
                else
                {
                    DVASSERT(false);
                }
            }
        }
    }
    else
    {
        DVASSERT(false);
    }
}

void UIPackageModel::RemoveNode(ControlNode *node)
{
    PackageBaseNode *parentNode = node->GetParent();
    if ((parentNode->GetFlags() & PackageBaseNode::FLAG_READ_ONLY) == 0)
    {
        PackageControlsNode *controls = dynamic_cast<PackageControlsNode*>(parentNode);
        if (controls)
        {
            controls->Remove(node);
        }
        else
        {
            ControlNode *parentControl = dynamic_cast<ControlNode*>(parentNode);
            if (parentControl)
            {
                parentControl->Remove(node);
            }
            else
            {
                DVASSERT(false);
            }
        }
    }
    else
    {
        DVASSERT(false);
    }
    
//    if (control->GetParent())
//    {
//        RemoveNodeFromContent(control);
//    }
//    else
//    {
//        RemoveNodeFromPackage(control);
//    }
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

