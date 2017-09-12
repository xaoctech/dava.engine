#pragma once

#include <Base/Vector.h>
#include <Scene3D/Entity.h>
#include "Physics/CollisionShapeComponent.h"

namespace DAVA
{
namespace PhysicsUtils
{
/** Get vector of collision components attached to the entity */
Vector<CollisionShapeComponent*> GetShapeComponents(Entity* entity);
}
}