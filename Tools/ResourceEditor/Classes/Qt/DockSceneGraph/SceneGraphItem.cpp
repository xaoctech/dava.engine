#include "SceneGraphItem.h"

SceneGraphItem::SceneGraphItem(GraphItem *parent)
	:	GraphItem(parent),
        extraUserData(NULL)
{
}

SceneGraphItem::~SceneGraphItem()
{
	ReleaseUserData();
    
    // User Data memory isn't controlled by us.
    extraUserData = NULL;
}

QVariant SceneGraphItem::Data(int32 column)
{
	DVASSERT((0 == column) && "Wrong column requested");

	if(userData)
	{
		Entity *node = (Entity *)userData;
		return QVariant(QString(node->GetName().c_str()));
	}

    if (extraUserData)
    {
        return QVariant(extraUserData->GetName());
    }

	return QVariant(QString("! NULL Node"));
}

void SceneGraphItem::SetUserData(void *data)
{
    ReleaseUserData();
    
	Entity *node = (Entity *)data;
	userData = SafeRetain(node);
}

void SceneGraphItem::SetExtraUserData(ExtraUserData* extraData)
{
    this->extraUserData = extraData;
}

ExtraUserData* SceneGraphItem::GetExtraUserData() const
{
    return this->extraUserData;
}

void SceneGraphItem::ReleaseUserData()
{
	Entity *node = (Entity *)userData;
	SafeRelease(node);
	userData = NULL;
}