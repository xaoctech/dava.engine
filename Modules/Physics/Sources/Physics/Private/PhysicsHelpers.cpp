#include "Physics/PhysicsHelpers.h"
#include "Physics/PhysicsModule.h"

#include <Engine/Engine.h>
#include <ModuleManager/ModuleManager.h>

namespace DAVA
{
CharacterControllerComponent* GetCharacterControllerComponent(Entity* entity)
{
    DVASSERT(entity != nullptr);

    const PhysicsModule* module = GetEngineContext()->moduleManager->GetModule<PhysicsModule>();
    DVASSERT(module != nullptr);

    const Vector<uint32>& characterControllerComponents = module->GetCharacterControllerComponentTypes();

    for (uint32 controllerType : characterControllerComponents)
    {
        CharacterControllerComponent* component = static_cast<CharacterControllerComponent*>(entity->GetComponent(controllerType));
        if (component != nullptr)
        {
            return component;
        }
    }

    return nullptr;
}
}