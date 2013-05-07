#ifndef __SCENE_TREE_ITEM_H__
#define __SCENE_TREE_ITEM_H__

#include <QStandardItem>

// framework
#include "Scene3D/Entity.h"

class SceneTreeItem : public QStandardItem
{
public:
	SceneTreeItem(DAVA::Entity *entity);
	~SceneTreeItem();

	int	type() const;
	QVariant data(int role) const;
	void setData(const QVariant & value, int role);

	DAVA::Entity* GetEntity() const;
	SceneTreeItem* SearchEntity(DAVA::Entity *entity);

protected:
	DAVA::Entity *entity;

	void UpdateEntity();
};

#endif // __QT_PROPERTY_ITEM_H__
