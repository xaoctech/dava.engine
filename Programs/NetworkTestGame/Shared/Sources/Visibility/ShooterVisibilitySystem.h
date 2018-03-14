#pragma once

#include "Visibility/ObserverComponent.h"
#include "Visibility/ObservableComponent.h"
#include "Visibility/CharacterVisibilityShapeComponent.h"

#include "NetworkCore/NetworkTypes.h"
#include "NetworkCore/Scene3D/Components/NetworkPlayerComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkTimeSingleComponent.h"
#include "Physics/Core/HeightFieldShapeComponent.h"

#include "Entity/SceneSystem.h"
#include "Scene3D/EntityGroup.h"
#include "Base/FastName.h"
#include "Base/BaseTypes.h"

#include "physx/geometry/PxHeightFieldGeometry.h"
#include "PxShared/foundation/PxTransform.h"

namespace DAVA
{
class Scene;
class Entity;
class TransformComponent;
}

class ShooterVisibilitySystem : public DAVA::SceneSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(ShooterVisibilitySystem, DAVA::SceneSystem);

    ShooterVisibilitySystem(DAVA::Scene* scene);
    ~ShooterVisibilitySystem() override;

    void PrepareForRemove() override{};
    void ProcessFixed(DAVA::float32 timeElapsed) override;
    void Process(DAVA::float32 timeElapsed) override;

    void EnableDebugRender(bool isEnabled);
    void EnableDebugLog(bool isEnabled);

private:
    struct ObserverItem
    {
        ObserverComponent* comp;
        DAVA::TransformComponent* transform;
        DAVA::Vector<DAVA::uint8> updatePeriodByObservableId;
    };

    struct ObservableItem
    {
        CharacterVisibilityShapeComponent* shape = nullptr; // null if simple observable
        DAVA::TransformComponent* transform;
    };

    struct ObserverCache
    {
        DAVA::Entity* entity;
        DAVA::Vector3 extrapolatedPOV;
        DAVA::Vector3 position;
        DAVA::float32 maxVisibilityRadiusSqr;
        DAVA::float32 unconditionalVisibilityRadiusSqr;
    };

    struct ObservableCache
    {
        DAVA::Entity* entity;
        DAVA::Vector3 tracePoint;
        DAVA::Vector3 position;
        ObservableId observableId;
        bool simpleObservable;
    };

    struct HeightFieldCache
    {
        physx::PxTransform pose;
        physx::PxHeightFieldGeometry geom;
    };

    void PrepareHeightFieldCaches();
    void PrepareObservableCaches();
    void PrepareObserverCache(ObserverCache& outCache, ObserverItem& observer);
    DAVA::Vector3 ComputeExtrapolatedPOV(const ObserverItem& observer);
    bool PreciseGeometricCheck(ObserverCache& observerCache, ObservableCache& observableCache);
    DAVA::uint8 ComputeVisibility(bool& outIsVisible, ObserverCache& observerCache, ObservableCache& observableCache);
    void ProcessDebugLog();

    void AddObserver(ObserverComponent* component);
    void RemoveObserver(ObserverComponent* component);
    void AddObservable(ObservableComponent* component);
    void RemoveObservable(ObservableComponent* component);

    DAVA::Vector<ObserverItem> observers;
    DAVA::UnorderedMap<ObservableId, ObservableItem> observables;
    DAVA::Vector<ObservableCache> toObserveOnCurrFrame;

    DAVA::ComponentGroup<ObserverComponent>* observerGroup;
    DAVA::ComponentGroup<ObservableComponent>* characterObservableGroup;
    DAVA::ComponentGroup<ObservableComponent>* simpleObservableGroup;

    DAVA::ComponentGroup<DAVA::HeightFieldShapeComponent>* heightFieldComponents;
    DAVA::Vector<HeightFieldCache> heightFiledCaches;

    DAVA::uint32 frameOffset = 0;

    bool enableDebugRender;
    DAVA::Vector<DAVA::Ray3> visibilityRays;
    bool enableDebugLog;
    DAVA::uint32 framesToDebugLog;
    DAVA::uint32 raycastCountTotal;
    DAVA::uint32 raycastCountCurrFrame;
    DAVA::uint32 maxRaycatCountPerFrame;
};
