#ifndef __SCENE_COLLISION_BASE_OBJECT_H__
#define __SCENE_COLLISION_BASE_OBJECT_H__

#include "bullet/btBulletCollisionCommon.h"
#include "Scene3D/Entity.h"

class CollisionBaseObject
{
public:
	CollisionBaseObject(DAVA::Entity *ent, btCollisionWorld *word)
		: entity(ent)
		, btWord(word)
		, btObject(NULL)
	{ }

	virtual ~CollisionBaseObject() = 0
	{ }

	DAVA::Entity *entity;
	DAVA::AABBox3 boundingBox;
	btCollisionObject *btObject;
	btCollisionWorld *btWord;
};

#endif // __SCENE_COLLISION_BASE_OBJECT_H__
