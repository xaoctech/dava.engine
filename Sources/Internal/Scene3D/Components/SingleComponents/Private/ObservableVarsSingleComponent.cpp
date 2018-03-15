#include "Scene3D/Components/SingleComponents/ObservableVarsSingleComponent.h"

#include "Reflection/ReflectionRegistrator.h"
#include "Reflection/ReflectedMeta.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(ObservableVarsSingleComponent)
{
    ReflectionRegistrator<ObservableVarsSingleComponent>::Begin()[M::CantBeCreatedManualyComponent(), M::Replicable(M::Privacy::PUBLIC)]
    .ConstructorByPointer()
    .End();
}

void ObservableVarsSingleComponent::NotifyAboutChanges(const IVar* var, Any&& value)
{
    changes[var] = value;
}

const UnorderedMap<const IVar*, Any>& ObservableVarsSingleComponent::GetChanges() const
{
    return changes;
}

void ObservableVarsSingleComponent::Clear()
{
    changes.clear();
}
}
