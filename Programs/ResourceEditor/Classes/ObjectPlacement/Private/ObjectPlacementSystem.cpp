#include "Classes/ObjectPlacement/Private/ObjectPlacementSystem.h"
#include "Classes/Qt/Scene/System/ModifSystem.h"

ObjectPlacementSystem::ObjectPlacementSystem(DAVA::Scene* scene)
    : DAVA::SceneSystem(scene)
{
    SceneEditor2* editorScene = static_cast<SceneEditor2*>(scene);
    modificationSystem = editorScene->modifSystem;
}

bool ObjectPlacementSystem::GetSnapToLandscape() const
{
    return snapToLandscape;
}

void ObjectPlacementSystem::SetSnapToLandscape(bool newSnapToLandscape)
{
    if (!GetScene())
        return;

    if (nullptr == landscape)
    {
        DAVA::Logger::Error(ResourceEditor::NO_LANDSCAPE_ERROR_MESSAGE.c_str());
        return;
    }
    snapToLandscape = newSnapToLandscape;
    DVASSERT(modificationSystem);
    modificationSystem->SetLandscapeSnap(snapToLandscape);
}

void ObjectPlacementSystem::PlaceOnLandscape() const
{
    if (!GetScene())
        return;

    if (nullptr == landscape)
    {
        DAVA::Logger::Error(ResourceEditor::NO_LANDSCAPE_ERROR_MESSAGE.c_str());
        return;
    }
    DVASSERT(modificationSystem);
    const SelectableGroup& selection = Selection::GetSelection();
    modificationSystem->PlaceOnLandscape(selection);
}

void ObjectPlacementSystem::AddEntity(DAVA::Entity* entity)
{
    if (GetLandscape(entity) != NULL)
    {
        landscape = GetLandscape(entity);
    }
}

void ObjectPlacementSystem::RemoveEntity(DAVA::Entity* entity)
{
    if (GetLandscape(entity) != NULL)
    {
        snapToLandscape = false;
        landscape = nullptr;
        DVASSERT(modificationSystem);
        modificationSystem->SetLandscapeSnap(snapToLandscape);
    }
}

void ObjectPlacementSystem::PlaceAndAlign() const
{
}
