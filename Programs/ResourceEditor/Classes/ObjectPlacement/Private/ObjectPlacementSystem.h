#pragma once

#include "Scene/System/SystemDelegates.h"
#include "Scene3D/Entity.h"
#include "UI/UIEvent.h"

#include "Scene/SceneTypes.h"
#include "Classes/Selection/SelectableGroup.h"

class ObjectPlacementSystem : public DAVA::SceneSystem, public EditorSceneSystem
{
public:
    ObjectPlacementSystem(DAVA::Scene* scene);
    void SetSnapToLandscape(bool newSnapToLandscape);
    bool GetSnapToLandscape() const;
    void PlaceOnLandscape() const;

    void AddEntity(DAVA::Entity* entity) override;
    void RemoveEntity(DAVA::Entity* entity) override;
private:
    // FIXME: pointer to scene modification system in ObjectPlacementSystem
    // As for now, Modification System is not reflective, doesn't
    // have its own TArc module or DataNode. So, to be able
    // to set Modification System's fields via Object Placement module's
    // reflective controls, we need to store it here.
    EntityModificationSystem* modificationSystem = nullptr;

    bool snapToLandscape = false;
    DAVA::Landscape* landscape = nullptr;
};
