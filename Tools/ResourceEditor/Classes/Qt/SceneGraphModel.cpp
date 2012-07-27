#include "SceneGraphModel.h"
#include "SceneGraphItem.h"


SceneGraphModel::SceneGraphModel(QObject *parent)
    : GraphModel(parent)
{
}

SceneGraphModel::~SceneGraphModel()
{
}


void SceneGraphModel::SetupModelData()
{
	SceneNode *rootNode = new SceneNode();
	rootNode->SetName(String("Scene Node Graph"));

	rootItem = new SceneGraphItem();
	rootItem->SetUserData(rootNode);
	SafeRelease(rootNode);


}