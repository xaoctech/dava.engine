#include "Scene3D/Components/SingleComponents/TransformSingleComponent.h"

#include "Reflection/ReflectionRegistrator.h"
#include "Utils/Utils.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(TransformSingleComponent)
{
    ReflectionRegistrator<TransformSingleComponent>::Begin()
    .ConstructorByPointer()
    .End();
}

namespace TSCDetails
{
void RemoveEntitiesFromVector(Vector<Entity*>& vector, const Entity* entity)
{
    size_t size = vector.size();
    size_t k = 0;
    while (k < size)
    {
        if (vector[k] == entity)
        {
            vector[k] = vector.back();
            vector.pop_back();
            --size;
        }
        else
        {
            ++k;
        }
    }
}
}

void TransformSingleComponent::Clear()
{
    localTransformChanged.clear();
    transformParentChanged.clear();
    worldTransformChanged.Clear();
    animationTransformChanged.clear();
}

void TransformSingleComponent::EraseEntity(const Entity* entity)
{
    TSCDetails::RemoveEntitiesFromVector(localTransformChanged, entity);
    TSCDetails::RemoveEntitiesFromVector(transformParentChanged, entity);
    TSCDetails::RemoveEntitiesFromVector(animationTransformChanged, entity);
    worldTransformChanged.EraseEntity(entity);
}
}