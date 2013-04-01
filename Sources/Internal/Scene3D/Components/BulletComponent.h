#ifndef __DAVAENGINE_BULLET_COMPONENT_H__
#define __DAVAENGINE_BULLET_COMPONENT_H__

#include "Base/BaseTypes.h"
#include "Entity/Component.h"
#include "Scene3D/Entity.h"

namespace DAVA
{

class BaseObject;
class BulletComponent : public Component
{
public:
	IMPLEMENT_COMPONENT_TYPE(BULLET_COMPONENT);

	BulletComponent();
	virtual ~BulletComponent();

	void SetBulletObject(BaseObject * bulletObject);
	BaseObject * GetBulletObject();

	virtual Component * Clone(Entity * toEntity);
	virtual void Serialize(KeyedArchive *archive, SceneFileV2 *sceneFile);
	virtual void Deserialize(KeyedArchive *archive, SceneFileV2 *sceneFile);

private:
	BaseObject * bulletObject;
    
public:
    INTROSPECTION_EXTEND(BulletComponent, Component,
        PROPERTY("bulletObject", "Bullet Object", GetBulletObject, SetBulletObject, INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
    );
};

}

#endif //__DAVAENGINE_BULLET_COMPONENT_H__
