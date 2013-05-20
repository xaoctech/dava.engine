#include "DockSceneTree/SceneTreeItem.h"


SceneTreeItem::SceneTreeItem(DAVA::Entity *_entity)
	: entity(_entity)
{
	UpdateEntity();
}

SceneTreeItem::~SceneTreeItem()
{ }

int SceneTreeItem::type() const
{
	return QStandardItem::UserType + 1;
}

QVariant SceneTreeItem::data(int role) const
{
	QVariant v;

	switch(role)
	{
	//case Qt::DecorationRole:
	//	v = itemData->GetIcon();
	//	break;
	case Qt::DisplayRole:
		v = entity->GetName().c_str();
		break;
	default:
		break;
	}

	if(v.isNull())
	{
		v = QStandardItem::data(role);
	}

	return v;
}

void SceneTreeItem::setData(const QVariant & value, int role)
{
	switch(role)
	{
	case Qt::EditRole:
		/*
		if(NULL != itemData)
		{
			itemData->SetValue(value);
		}
		break;
		*/
	default:
		QStandardItem::setData(value, role);
		break;
	}
}

DAVA::Entity* SceneTreeItem::GetEntity() const
{
	return entity;
}

SceneTreeItem* SceneTreeItem::SearchEntity(DAVA::Entity *entityToSearch)
{
	SceneTreeItem * ret = NULL;

	if(entityToSearch == entity)
	{
		ret = this;
	}
	else
	{
		for (int i = 0; i < rowCount(); i++)
		{
			SceneTreeItem *childItem = (SceneTreeItem *) child(i);

			ret = childItem->SearchEntity(entityToSearch);
			if(NULL != ret)
			{
				break;
			}
		}
	}

	return ret;
}

void SceneTreeItem::UpdateEntity()
{
	if(NULL != entity)
	{
		// remove all item children
		removeRows(0, rowCount());

		// check if entity has children
		if(entity->GetChildrenCount() > 0)
		{
			// if entity is solid - just mark it
			if(!entity->GetSolid())
			{
				// add child items
				for (int i = 0; i < entity->GetChildrenCount(); ++i)
				{
					appendRow(new SceneTreeItem(entity->GetChild(i)));
				}
			}
		}
	}
}
