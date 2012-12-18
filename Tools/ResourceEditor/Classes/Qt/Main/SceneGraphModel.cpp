#include "SceneGraphModel.h"
#include "SceneGraphItem.h"

#include "Qt/Scene/SceneData.h"
#include "Qt/Scene/SceneDataManager.h"
#include "../EditorScene.h"

#include "GraphItem.h"
#include "PointerHolder.h"
#include "../SceneEditor/SceneEditorScreenMain.h"

#include "DockParticleEditor/ParticlesEditorController.h"

#include <QTreeView>

using namespace DAVA;

SceneGraphModel::SceneGraphModel(QObject *parent)
    :   GraphModel(parent)
    ,   scene(NULL)
    ,   selectedNode(NULL)
    ,   selectedGraphItemForParticleEditor(NULL)
{
}

SceneGraphModel::~SceneGraphModel()
{
    SafeRelease(scene);
}

void SceneGraphModel::SetScene(EditorScene *newScene)
{
    SafeRelease(scene);
    scene = SafeRetain(newScene);
 
    Rebuild();
}

void SceneGraphModel::AddNodeToTree(GraphItem *parent, DAVA::SceneNode *node)
{
    // Particles Editor can change the node type during "adopting" Particle Emitter Nodes.
    node = particlesEditorSceneHelper.PreprocessSceneNode(node);

    SceneGraphItem *graphItem = new SceneGraphItem();
	graphItem->SetUserData(node);
    parent->AppendChild(graphItem);
    
    // Particles Editor nodes are processed separately.
    if (particlesEditorSceneHelper.AddNodeToSceneGraph(graphItem, node))
    {
        // Also try to determine selected item from Particle Editor.
        SceneGraphItem *selectedItem = particlesEditorSceneHelper.GetGraphItemToBeSelected(graphItem, node);
        if (selectedItem)
        {
            this->selectedGraphItemForParticleEditor = selectedItem;
        }

        return;
    }

    if(!node->GetSolid())
    {
        int32 count = node->GetChildrenCount();
        for(int32 i = 0; i < count; ++i)
        {
            AddNodeToTree(graphItem, node->GetChild(i));
        }
    }
}

void SceneGraphModel::Rebuild()
{
    SafeRelease(rootItem);
	rootItem = new SceneGraphItem();
	rootItem->SetUserData(scene);
    
    this->selectedGraphItemForParticleEditor = NULL;
    
    if(scene && !scene->GetSolid())
    {
        int32 count = scene->GetChildrenCount();
        for(int32 i = 0; i < count; ++i)
        {
            AddNodeToTree(rootItem, scene->GetChild(i));
        }
    }
    
//    emit dataChanged(QModelIndex(), QModelIndex());
    this->reset();

    if (HandleParticleEditorSelection())
    {
        // Custom node is selected and needs to be processed separately.
        return;
    }
    
    if(selectedNode)
    {
        GraphItem *selectedItem = ItemForData(selectedNode);
        if(selectedItem)
        {
            SelectItem(selectedItem);
        }
        else
        {
            SelectNode(NULL);
        }
    }
}

bool SceneGraphModel::HandleParticleEditorSelection()
{
    if (this->selectedGraphItemForParticleEditor == NULL)
    {
        return false;
    }

    SelectItem(this->selectedGraphItemForParticleEditor, true);
    return true;
}

void SceneGraphModel::SelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    if(LandscapeEditorModeEnabled())
        return;
    
    int32 deselectedSize = deselected.size();
    int32 selectedSize = selected.size();
    
    DVASSERT((selectedSize <= 1) && "Wrong count of selected items");
    DVASSERT((deselectedSize <= 1) && "Wrong count of deselected items");

    // Particles Editor Nodes are handled separately.
    if (particlesEditorSceneHelper.ProcessSelectionChanged(selected, deselected))
    {
        return;
    }

    if(0 < selectedSize)
    {
        QItemSelectionRange selectedRange = selected.at(0);
        SceneGraphItem *selectedItem = static_cast<SceneGraphItem*>(selectedRange.topLeft().internalPointer());
        SceneNode *newSelectedNode = static_cast<SceneNode *>(selectedItem->GetUserData());

        if(0 < deselectedSize)
        {
            QItemSelectionRange deselectedRange = deselected.at(0);
            SceneGraphItem *deselectedItem = static_cast<SceneGraphItem*>(deselectedRange.topLeft().internalPointer());
            if(deselectedItem->GetUserData() == selectedItem->GetUserData())
            {
                Logger::Warning("Select the same node");
            }
        }
        SelectNode(newSelectedNode, false);
    }
    else
    {
        SelectNode(NULL, false);
    }
}

void SceneGraphModel::SelectNode(DAVA::SceneNode *node)
{
    SelectNode(node, true);
}

void SceneGraphModel::SelectNode(DAVA::SceneNode *node, bool selectAtGraph)
{
    if(selectedNode != node)
    {
        selectedNode = node;
        emit SceneNodeSelected(selectedNode);
    }
    
    if(selectAtGraph)
    {
        if(selectedNode)
        {
            GraphItem *selectedItem = ItemForData(selectedNode);
            if(selectedItem)
            {
                SelectItem(selectedItem);
            }
            else
            {
                Rebuild();
            }

        }
        else
        {
            itemSelectionModel->clearSelection();
        }
    }
}

void SceneGraphModel::SelectItem(GraphItem *item, bool needExpand)
{
    QModelIndex idx = createIndex(item->Row(), 0, item);
    itemSelectionModel->select(idx, QItemSelectionModel::ClearAndSelect);
    
    if(attachedTreeView)
    {
        attachedTreeView->scrollTo(idx);
        if (needExpand)
        {
            attachedTreeView->expand(idx);
        }
    }
}

DAVA::SceneNode * SceneGraphModel::GetSelectedNode()
{
    return selectedNode;
}

Qt::ItemFlags SceneGraphModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
	{
		return Qt::ItemIsDropEnabled;
	}
    
    if(LandscapeEditorModeEnabled())
    {
        return Qt::ItemIsEnabled;
    }

    return (Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | GraphModel::flags(index));
}

bool SceneGraphModel::LandscapeEditorModeEnabled() const
{
    SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
    if(screen)
    {
        return screen->LandscapeEditorModeEnabled();
    }
    
    return false;
}

Qt::DropActions SceneGraphModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;    
}

bool SceneGraphModel::removeRows(int row, int count, const QModelIndex &parent/* = QModelIndex()*/)
{
    DVASSERT((0 < count) && "Wrong count of removed rows");
    
    Logger::Warning("[SceneGraphModel::removeRows] why?");
    

//    GraphItem *parentItem = rootItem;
//    if(parent.isValid())
//    {
//        parentItem = static_cast<GraphItem *>(parent.internalPointer());
//    }
//    
//
//    int32 firstRow = row;
//    int32 lastRow = row + count - 1;
//    beginRemoveRows(parent, firstRow, lastRow);
//    
//    SceneNode *parentNode = static_cast<SceneNode *>(parentItem->GetUserData());
//    if(parentNode)
//    {
//        int32 lastNodeRow = Min(lastRow, parentNode->GetChildrenCount() - 1);
//        for(int32 i = firstRow; i <= lastNodeRow; ++i)
//        {
//            SceneNode *removedNode = parentNode->GetChild(i);
//            parentNode->RemoveNode(removedNode);
//            Logger::Debug("[%d] Remove %s from %s", i, removedNode->GetName().c_str(), parentNode->GetName().c_str());
//        }
//        
//        int32 lastItemRow = Min(lastRow, parentItem->ChildrenCount() - 1);
//        for(int32 i = firstRow; i <= lastItemRow; ++i)
//        {
//            parentItem->RemoveChild(i);
//        }
//    }
//    
//    endRemoveRows();
    
    return true;
}

bool SceneGraphModel::insertRows(int row, int count, const QModelIndex &parent/* = QModelIndex()*/)
{
    DVASSERT((0 < count) && "Wrong count of inserted rows");

    GraphItem *parentItem = rootItem;
    if(parent.isValid())
    {
        parentItem = static_cast<GraphItem *>(parent.internalPointer());
    }


    int32 firstRow = row;
    int32 lastRow = row + count - 1;
    beginInsertRows(parent, firstRow, lastRow);

    for(int32 i = firstRow; i <= lastRow; ++i)
    {
        SceneGraphItem *graphItem = new SceneGraphItem();
        graphItem->SetUserData(NULL);
        parentItem->AppendChild(graphItem);
    }
    
    
    endInsertRows();
    return true;
}


bool SceneGraphModel::setData(const QModelIndex &index, const QVariant &value, int role/* = Qt::EditRole*/)
{
    GraphItem *item = static_cast<GraphItem*>(index.internalPointer());
    
    //TODO: change on real value
    GraphItem *newItem = PointerHolder<GraphItem *>::ToPointer(value);
    SceneNode *newNode = (SceneNode *)newItem->GetUserData();
    SafeRetain(newNode);
    
    if(newNode && newNode->GetParent())
    {
        newNode->GetParent()->RemoveNode(newNode);
    }
    
    GraphItem *parentItem = item->GetParent();
    SceneNode *parentNode = static_cast<SceneNode *>(parentItem->GetUserData());
    if(parentNode)
    {
        parentNode->AddNode(newNode);
    }
    
    item->SetUserData(newNode);
    SafeRelease(newNode);
    
    return true;
}

void SceneGraphModel::MoveItemToParent(GraphItem * movedItem, const QModelIndex &newParentIndex)
{
    GraphItem *newParentItem = rootItem;
    if(newParentIndex.isValid())
    {
        newParentItem = static_cast<GraphItem*>(newParentIndex.internalPointer());
    }
    

    GraphItem *oldParentItem = movedItem->GetParent();
    
    oldParentItem->RemoveChild(movedItem);
    newParentItem->AppendChild(movedItem);
    
    SceneNode *movedNode = static_cast<SceneNode *>(movedItem->GetUserData());
    SceneNode *newParentNode = static_cast<SceneNode *>(newParentItem->GetUserData());
    SceneNode *oldParentNode = static_cast<SceneNode *>(oldParentItem->GetUserData());

    DVASSERT((NULL != movedNode) && "movedNode is NULL");
    DVASSERT((NULL != newParentNode) && "newParentNode is NULL");
    DVASSERT((NULL != oldParentNode) && "oldParentNode is NULL");
    
    SafeRetain(movedNode);
    oldParentNode->RemoveNode(movedNode);
    newParentNode->AddNode(movedNode);
    SafeRelease(movedNode);
}

QVariant SceneGraphModel::data(const QModelIndex &index, int role) const
{
    if(index.isValid() && (Qt::TextColorRole == role))
    {
        SceneNode *node = static_cast<SceneNode *>(ItemData(index));
        if(node && (node->GetFlags() & SceneNode::NODE_INVALID))
        {
            return QColor(255, 0, 0);
        }
    }
    
    return GraphModel::data(index, role);
}





