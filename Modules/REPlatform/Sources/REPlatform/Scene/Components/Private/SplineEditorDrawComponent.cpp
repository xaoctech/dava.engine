#include "REPlatform/Scene/Components/SplineEditorDrawComponent.h"

#include <REPlatform/Global/REMeta.h>

#include <Reflection/ReflectionRegistrator.h>
#include <Reflection/ReflectedMeta.h>

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(SplineEditorDrawComponent)
{
    ReflectionRegistrator<SplineEditorDrawComponent>::Begin()[M::NonExportableComponent(),
                                                              M::NonSerializableComponent(),
                                                              M::HiddenField(),
                                                              M::CantBeCreatedManualyComponent(),
                                                              M::CantBeDeletedManualyComponent(),
                                                              M::DisableEntityReparent()]
    .End();
}

bool SplineEditorDrawComponent::operator==(const SplineEditorDrawComponent& other) const
{
    return (isSelected == other.isSelected) && (isSnapped == other.isSnapped);
}

template <>
bool AnyCompare<SplineEditorDrawComponent>::IsEqual(const Any& v1, const Any& v2)
{
    return v1.Get<SplineEditorDrawComponent>() == v2.Get<SplineEditorDrawComponent>();
}

} // ns DAVA
