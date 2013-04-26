#ifndef __ENTITY_GROUP_H__
#define __ENTITY_GROUP_H__

#include "Scene3D/Entity.h"

struct EntityGroupItem 
{
	EntityGroupItem() : entity(NULL), solidEntity(NULL)	
	{ }

	EntityGroupItem(DAVA::Entity *_entity, DAVA::Entity *_solidEntity, DAVA::AABBox3 _bbox) 
		: entity(_entity), solidEntity(_solidEntity), bbox(_bbox)
	{ }

	DAVA::Entity *entity;
	DAVA::Entity *solidEntity;
	DAVA::AABBox3 bbox;
};

class EntityGroup
{
public:
	EntityGroup();
	EntityGroup(const EntityGroup &ss);
	~EntityGroup();

	void Add(DAVA::Entity *entity, DAVA::Entity *solidEntity, DAVA::AABBox3 entityBbox);
	void Add(const EntityGroupItem &groupItem);
	void Rem(DAVA::Entity *entity);
	void Clear();

	size_t Size() const;
	DAVA::Entity* GetEntity(size_t i) const;
	DAVA::Entity* GetSolidEntity(size_t i) const;
	EntityGroupItem GetItem(size_t i) const;
	DAVA::AABBox3 GetBbox(size_t i) const;
	DAVA::AABBox3 GetCommonBbox() const;

	bool HasEntity(DAVA::Entity *entity) const;
	bool Index(DAVA::Entity *entity, size_t &index) const;

	DAVA::Entity* IntersectedEntity(const EntityGroup *group) const;

	EntityGroup& operator=(const EntityGroup &ss);
	bool operator==(const EntityGroup &ss) const;

protected:
	DAVA::Vector<EntityGroupItem> entities;
	DAVA::AABBox3 entitiesBbox;
};

#endif // __ENTITY_GROUP_H__
