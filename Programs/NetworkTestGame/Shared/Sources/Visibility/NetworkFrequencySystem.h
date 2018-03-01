#pragma once

#include "ObserverComponent.h"
#include "ObservableComponent.h"

#include "NetworkCore/Scene3D/Components/NetworkPlayerComponent.h"

#include "Entity/SceneSystem.h"
#include "Scene3D/EntityGroup.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Base/BaseTypes.h"

class NetworkFrequencySystem : public DAVA::SceneSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(NetworkFrequencySystem, DAVA::SceneSystem);

    NetworkFrequencySystem(DAVA::Scene* scene);
    ~NetworkFrequencySystem() override;

    void ProcessFixed(DAVA::float32 timeElapsed) override;

    void SetMaxAOI(DAVA::float32 maxAOI);
    DAVA::float32 GetMaxAOI() const;

    void SetNetworkPeriodIncreaseDistance(DAVA::float32 nwPeriodIncreaseDistance);
    DAVA::float32 GetNetworkPeriodIncreaseDistance() const;

    void SetUpdatePeriod(DAVA::uint8 period);
    DAVA::uint8 GetUpdatePeriod() const;

private:
    struct ObserverItem
    {
        ObserverComponent* comp;
        DAVA::TransformComponent* transform;
        DAVA::NetworkPlayerComponent* playerComp;
        DAVA::Vector<DAVA::uint8> networkPeriodByObservableId;
    };

    struct ObserverCache
    {
        DAVA::Vector3 position;
        DAVA::NetworkPlayerComponent* playerComp;
        DAVA::Vector<DAVA::uint8>* networkPeriodByObservableId;
    };

    struct ObservableCache
    {
        DAVA::Vector3 position;
        DAVA::Entity* entity;
        ObservableId id;
    };

    void UpdateNetworkPeriod(ObserverCache& observerCache, ObservableCache& observableCache, bool isVisible);
    void Hide(ObserverCache& observerCache, ObservableCache& observableCache);

    void AddObserver(ObserverComponent* component);
    void RemoveObserver(ObserverComponent* component);
    void AddObservable(ObservableComponent* component);
    void RemoveObservable(ObservableComponent* component);

    DAVA::Vector<ObserverItem> observers;
    DAVA::UnorderedMap<ObservableId, DAVA::TransformComponent*> observableTransforms;
    DAVA::Vector<ObservableCache> toObserveOnCurrFrame;

    DAVA::ComponentGroup<ObserverComponent>* observerGroup;
    DAVA::ComponentGroup<ObservableComponent>* observableGroup;

    DAVA::uint8 updatePeriod;
    DAVA::uint8 frameOffset = 0;

    DAVA::float32 maxAOISqr = 1e12f;
    DAVA::float32 nwPeriodIncreaseDistanceSqr = 1e10f;
};
