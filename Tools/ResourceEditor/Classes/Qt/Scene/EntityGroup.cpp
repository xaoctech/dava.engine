#include "Scene/EntityGroup.h"


EntityGroup::EntityGroup()
{ }

EntityGroup::EntityGroup(const EntityGroup &ss)
{
	entities = ss.entities;
}

EntityGroup::~EntityGroup()
{ }

void EntityGroup::Add(DAVA::Entity *entity, DAVA::AABBox3 entityBbox /* = DAVA::AABBox3() */)
{
	size_t i;
	if(!Index(entity, i))
	{
		EntityItem item;
		item.entity = entity;
		item.bbox = entityBbox;

		entities.push_back(item);
		entitiesBbox.AddAABBox(entityBbox);
	}
}

void EntityGroup::Rem(DAVA::Entity *entity)
{
	size_t i;
	if(Index(entity, i))
	{
		DAVA::Vector<EntityItem>::iterator it = entities.begin();
		entities.erase(it + i);

		// recalc common ab
		entitiesBbox.Empty();
		for(size_t j = 0; j < entities.size(); ++j)
		{
			entitiesBbox.AddAABBox(entities[j].bbox);
		}
	}

}

void EntityGroup::Clear()
{
	entities.clear();
	entitiesBbox.Empty();
}

size_t EntityGroup::Size() const
{
	return entities.size();
}

DAVA::Entity* EntityGroup::Get(size_t i) const
{
	DAVA::Entity *ret = NULL;

	if(i < entities.size())
	{
		ret = entities[i].entity;
	}

	return ret;
}

DAVA::AABBox3 EntityGroup::GetBbox(size_t i) const
{
	DAVA::AABBox3 ret;

	if(i < entities.size())
	{
		ret = entities[i].bbox;
	}

	return ret;
}

DAVA::AABBox3 EntityGroup::GetCommonBbox() const
{
	return entitiesBbox;
}

bool EntityGroup::HasEntity(DAVA::Entity *entity) const
{
	size_t i;
	return Index(entity, i);
}

EntityGroup& EntityGroup::operator=( const EntityGroup &ss )
{
	entities = ss.entities;
	return *this;
}

bool EntityGroup::operator==( const EntityGroup &ss ) const
{
	bool ret = false;

	if(entities.size() == ss.entities.size())
	{
		ret = true;
		for(size_t i = 0; i < entities.size(); ++i)
		{
			if(!ss.HasEntity(entities[i].entity))
			{
				ret = false;
				break;
			}
		}
	}

	return ret = true;
}

bool EntityGroup::Index(DAVA::Entity *entity, size_t &index) const
{
	for(size_t i = 0; i < entities.size(); ++i)
	{
		if(entities[i].entity == entity)
		{
			index = i;
			return true;
		}
	}

	return false;
}
