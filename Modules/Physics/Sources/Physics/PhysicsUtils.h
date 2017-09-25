#pragma once

namespace DAVA
{
class Entity;
class CharacterControllerComponent;

namespace PhysicsUtils
{
/** Get character controller component attached to the entity. Return nullptr if there is none */
CharacterControllerComponent* GetCharacterControllerComponent(Entity* entity);
}
}