#pragma once

#include <Base/Vector.h>

#include <physx/PxQueryFiltering.h>

namespace DAVA
{
class Entity;
class Scene;
class PhysicsComponent;
class CollisionShapeComponent;
class CharacterControllerComponent;
class Vector3;

namespace PhysicsUtils
{
/** Get static or dynamic body component attached to the entity. Return nullptr if there is none */
PhysicsComponent* GetBodyComponent(Entity* entity);

/** Get vector of collision components attached to the entity */
Vector<CollisionShapeComponent*> GetShapeComponents(Entity* entity);

/** Get character controller component attached to the entity. Return nullptr if there is none */
CharacterControllerComponent* GetCharacterControllerComponent(Entity* entity);

/** Create character mirror entity based on controller component **/
Entity* CreateCharacterMirror(CharacterControllerComponent* controllerComponent);
}
}
