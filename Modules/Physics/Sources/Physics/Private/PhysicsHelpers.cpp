#include "Physics/PhysicsHelpers.h"
#include "Physics/PhysicsModule.h"

#include <Engine/Engine.h>
#include <ModuleManager/ModuleManager.h>

namespace DAVA
{
Vector<CollisionShapeComponent*> GetShapeComponents(Entity* entity)
{
    const PhysicsModule* module = GetEngineContext()->moduleManager->GetModule<PhysicsModule>();
    const Vector<uint32>& shapeComponents = module->GetShapeComponentTypes();

    Vector<CollisionShapeComponent*> shapes;
    for (uint32 shapeType : shapeComponents)
    {
        const size_t shapesCount = entity->GetComponentCount(shapeType);
        if (shapesCount > 0)
        {
            for (int i = 0; i < shapesCount; ++i)
            {
                CollisionShapeComponent* component = static_cast<CollisionShapeComponent*>(entity->GetComponent(shapeType, i));
                DVASSERT(component != nullptr);

                shapes.push_back(component);
            }
        }
    }

    return shapes;
}
}