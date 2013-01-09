#include "Entity/Component.h"
#include "Scene3D/SceneNode.h"
#include "Debug/DVAssert.h"

namespace DAVA
{

Component::Component()
:	entity(0)
{

}

void Component::SetEntity(SceneNode * _entity)
{
	entity = _entity;
}

}