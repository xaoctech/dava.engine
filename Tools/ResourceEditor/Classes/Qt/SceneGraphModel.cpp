#include "SceneGraphModel.h"
#include "SceneGraphItem.h"

#include "SceneData.h"
#include "SceneDataManager.h"
#include "../EditorScene.h"

#include <QTreeView>

using namespace DAVA;

SceneGraphModel::SceneGraphModel(QObject *parent)
    :   GraphModel(parent)
    ,   scene(NULL)
    ,   selectedNode(NULL)
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
    SceneGraphItem *graphItem = new SceneGraphItem();
	graphItem->SetUserData(node);
    parent->AppendChild(graphItem);
    
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
    selectedNode = NULL;
    
    SafeRelease(rootItem);
	rootItem = new SceneGraphItem();
	rootItem->SetUserData(scene);
    
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
}

void SceneGraphModel::SelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    int32 deselectedSize = deselected.size();
    int32 selectedSize = selected.size();
    
    DVASSERT((selectedSize <= 1) && "Wrong count of selected items");
    DVASSERT((deselectedSize <= 1) && "Wrong count of deselected items");

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
            
            QModelIndex idx = createIndex(selectedItem->Row(), 0, selectedItem);
            itemSelectionModel->select(idx, QItemSelectionModel::ClearAndSelect);
            
            if(attachedTreeView)
            {
                attachedTreeView->scrollTo(idx);
            }
        }
        else
        {
            itemSelectionModel->clearSelection();
        }
    }
}


DAVA::SceneNode * SceneGraphModel::GetSelectedNode()
{
    return selectedNode;
}
