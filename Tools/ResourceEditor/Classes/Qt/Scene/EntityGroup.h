#ifndef __ENTITY_GROUP_H__
#define __ENTITY_GROUP_H__

#include "Scene3D/Entity.h"

class EntityGroup
{
public:
	EntityGroup();
	EntityGroup(const EntityGroup &ss);
	~EntityGroup();

	void Add(DAVA::Entity *entity, DAVA::AABBox3 entityBbox);
	void Rem(DAVA::Entity *entity);
	void Clear();

	size_t Size() const;
	DAVA::Entity* Get(size_t i) const;

	bool HasEntity(DAVA::Entity *entity) const;

	DAVA::AABBox3 GetBbox(size_t i) const;
	DAVA::AABBox3 GetCommonBbox() const;

	EntityGroup& operator=(const EntityGroup &ss);
	bool operator==(const EntityGroup &ss) const;

protected:
	struct EntityItem 
	{
		DAVA::Entity *entity;
		DAVA::AABBox3 bbox;
	};

	DAVA::Vector<EntityItem> entities;
	DAVA::AABBox3 entitiesBbox;

	bool Index(DAVA::Entity *entity, size_t &index) const;
};

#endif // __ENTITY_GROUP_H__
