#pragma once

#include "Visibility/ObservableComponent.h"
#include "Visibility/ObservableIdComponent.h"

#include "Entity/SceneSystem.h"
#include "Scene3D/EntityGroup.h"
#include "Scene3D/Entity.h"
#include "Base/BaseTypes.h"

class ObservableIdSystem : public DAVA::SceneSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(ObservableIdSystem, DAVA::SceneSystem);

    ObservableIdSystem(DAVA::Scene* scene);
    ~ObservableIdSystem() override;

    void PrepareForRemove() override{};
    void ProcessFixed(DAVA::float32 timeElapsed) override;

private:
    void AddObservable(DAVA::Entity* entity);
    void RemoveObservable(DAVA::Entity* entity);

    ObservableId GenerateObservableId();
    void FreeObservableId(ObservableId observableId);

    DAVA::Vector<DAVA::Entity*> addedEntities;
    DAVA::Vector<ObservableId> freeObservableIds;
    ObservableId nextNewObservableId;

    DAVA::EntityGroup* observableEntityGroup;
};
