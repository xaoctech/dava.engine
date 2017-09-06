#pragma once

#include <Base/Vector.h>
#include <Scene3D/Entity.h>
#include "Physics/CharacterControllerComponent.h"

namespace DAVA
{
/** Get character controller component attached to the entity. Return nullptr if there is none */
CharacterControllerComponent* GetCharacterControllerComponent(Entity* entity);
}