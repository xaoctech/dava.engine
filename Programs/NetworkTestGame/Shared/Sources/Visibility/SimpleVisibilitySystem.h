#pragma once

#include "Visibility/ObserverComponent.h"
#include "Visibility/ObservableComponent.h"

#include "Entity/SceneSystem.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Base/BaseTypes.h"

class SimpleVisibilitySystem : public DAVA::SceneSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(SimpleVisibilitySystem, DAVA::SceneSystem);

    SimpleVisibilitySystem(DAVA::Scene* scene);
    ~SimpleVisibilitySystem() override;

    void PrepareForRemove() override{};
    void ProcessFixed(DAVA::float32 timeElapsed) override;

    void SetUpdatePeriod(DAVA::uint8 period);
    DAVA::uint8 GetUpdatePeriod() const;

private:
    struct ObserverItem
    {
        ObserverComponent* comp;
        DAVA::TransformComponent* transform;
    };

    struct ObservableItem
    {
        DAVA::TransformComponent* transform;
        ObservableId id;
    };

    void AddObserver(ObserverComponent* component);
    void RemoveObserver(ObserverComponent* component);
    void AddObservable(ObservableComponent* component);
    void RemoveObservable(ObservableComponent* component);

    DAVA::Vector<ObserverItem> observers;
    DAVA::Vector<ObservableItem> observables;
    DAVA::Vector<ObservableItem> justAddedObservables;

    DAVA::ComponentGroup<ObserverComponent>* observerGroup;
    DAVA::ComponentGroup<ObservableComponent>* observableGroup;

    DAVA::uint8 updatePeriod;
    DAVA::uint8 frameOffset = 0;
};
