#include "SceneGraphItem.h"

SceneGraphItem::SceneGraphItem(GraphItem *parent)
	:	GraphItem(parent)
{
}

SceneGraphItem::~SceneGraphItem()
{
	ReleaseUserData();
}

QVariant SceneGraphItem::Data(int32 column)
{
	DVASSERT((0 == column) && "Wrong column requested");

	if(userData)
	{
		SceneNode *node = (SceneNode *)userData;
		return QVariant(node->GetName().c_str());
	}

	return QVariant();
}

void SceneGraphItem::SetUserData(void *data)
{
	SceneNode *node = (SceneNode *)data;
	userData = SafeRetain(node);
}

void SceneGraphItem::ReleaseUserData()
{
	SceneNode *node = (SceneNode *)userData;
	SafeRelease(node);
	userData = NULL;
}