#include "SceneGraphModel.h"
#include "SceneGraphItem.h"

using namespace DAVA;

SceneGraphModel::SceneGraphModel(QObject *parent)
    :   GraphModel(parent)
    ,   scene(NULL)
{
}

SceneGraphModel::~SceneGraphModel()
{
    SafeRelease(scene);
}

void SceneGraphModel::SetScene(DAVA::Scene *newScene)
{
    SafeRelease(rootItem);
    SafeRelease(scene);
    scene = SafeRetain(newScene);
    
	rootItem = new SceneGraphItem();
	rootItem->SetUserData(scene);
    
    if(!scene->GetSolid())
    {
        int32 count = scene->GetChildrenCount();
        for(int32 i = 0; i < count; ++i)
        {
            AddNodeToTree(rootItem, scene->GetChild(i));
        }
    }
}

void SceneGraphModel::AddNodeToTree(GraphItem *parent, DAVA::SceneNode *node)
{
    SceneGraphItem *graphItem = new SceneGraphItem(parent);
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

