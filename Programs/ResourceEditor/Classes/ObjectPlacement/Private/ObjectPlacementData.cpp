#include "Classes/ObjectPlacement/Private/ObjectPlacementData.h"
#include "Classes/Qt/Scene/System/ModifSystem.h"

const char* ObjectPlacementData::snapToLandscapePropertyName = "snapToLandscape";

bool ObjectPlacementData::GetSnapToLandscape() const
{
    DVASSERT(objectPlacementSystem);
    return objectPlacementSystem->GetSnapToLandscape();
}

void ObjectPlacementData::SetSnapToLandscape(bool newSnapToLandscape)
{
    DVASSERT(objectPlacementSystem);
    objectPlacementSystem->SetSnapToLandscape(newSnapToLandscape);
}

DAVA_VIRTUAL_REFLECTION_IMPL(ObjectPlacementData)
{
    DAVA::ReflectionRegistrator<ObjectPlacementData>::Begin()
    .Field(snapToLandscapePropertyName, &ObjectPlacementData::GetSnapToLandscape, nullptr)
    .End();
}
