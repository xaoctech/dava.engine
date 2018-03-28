#include "ShooterVisibilitySystem.h"
#include "CharacterVisibilityShapeComponent.h"
#include "ShooterConstants.h"
#include "Components/ShooterAimComponent.h"
#include "Visibility/SimpleVisibilityShapeComponent.h"

#include "Physics/PhysicsSystem.h"
#include "NetworkCore/Scene3D/Components/NetworkReplicationComponent.h"

#include "Debug/ProfilerCPU.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Scene.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Render/Highlevel/RenderSystem.h"
#include "Logger/Logger.h"
#include "Render/RenderHelper.h"

#include "physx/geometry/PxGeometryQuery.h"
#include "physx/PxShape.h"

#include <algorithm>

using namespace DAVA;

namespace ShooterVisibilityDetail
{
constexpr uint8 NEAR_ZONE_UDPATE_PERIOD = 32u;
constexpr uint8 FAR_ZONE_UDPATE_PERIOD = 4u;
constexpr uint8 RAYCAST_ZONE_VISIBLE_UDPATE_PERIOD = 32u;
constexpr uint8 RAYCAST_ZONE_INVISIBLE_UDPATE_PERIOD = 4u;
constexpr uint8 SIMPLE_OBSERVABLE_UDPATE_PERIOD = 4u;
constexpr float32 POV_EXTRAPOLATION_RATIO = 0.8f;

constexpr bool ENABLE_DEBUG_RENDER_BY_DEFAULT = false;
constexpr bool ENABLE_DEBUG_LOG_BY_DEFAULT = false;
constexpr uint32 DEBUG_LOG_PERIOD = 100u;

// Terrain filter
class VisibilityFilterCallback final : public physx::PxQueryFilterCallback
{
public:
    physx::PxQueryHitType::Enum preFilter(
    const physx::PxFilterData& filterData, const physx::PxShape* shape,
    const physx::PxRigidActor* actor, physx::PxHitFlags& queryFlags) override;

    physx::PxQueryHitType::Enum postFilter(
    const physx::PxFilterData& filterData, const physx::PxQueryHit& hit) override;
};

physx::PxQueryHitType::Enum VisibilityFilterCallback::preFilter(
const physx::PxFilterData& filterData, const physx::PxShape* shape,
const physx::PxRigidActor* actor, physx::PxHitFlags& queryFlags)
{
    bool isTerrainHit = (shape->getGeometryType() == physx::PxGeometryType::eHEIGHTFIELD);
    return isTerrainHit ? physx::PxQueryHitType::eBLOCK : physx::PxQueryHitType::eNONE;
}

physx::PxQueryHitType::Enum VisibilityFilterCallback::postFilter(
const physx::PxFilterData& filterData, const physx::PxQueryHit& hit)
{
    return physx::PxQueryHitType::eBLOCK;
}

inline physx::PxVec3 Vector3ToPxVec3(const Vector3& in)
{
    return physx::PxVec3(in.x, in.y, in.z);
}

} // namespace ShooterVisibilityDetail

DAVA_VIRTUAL_REFLECTION_IMPL(ShooterVisibilitySystem)
{
    ReflectionRegistrator<ShooterVisibilitySystem>::Begin()[M::Tags("server", "gm_shooter")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &ShooterVisibilitySystem::ProcessFixed)[M::SystemProcess(SP::Group::GAMEPLAY, SP::Type::FIXED, 999.3f)]
    .Method("Process", &ShooterVisibilitySystem::Process)[M::SystemProcess(SP::Group::GAMEPLAY, SP::Type::NORMAL, 999.f)]
    .End();
}

ShooterVisibilitySystem::ShooterVisibilitySystem(Scene* scene)
    : SceneSystem(scene, ComponentMask())
    , observerGroup(scene->AquireComponentGroup<ObserverComponent, ObserverComponent>())
    , characterObservableGroup(scene->AquireComponentGroup<ObservableComponent, ObservableComponent, ObservableIdComponent, CharacterVisibilityShapeComponent>())
    , simpleObservableGroup(scene->AquireComponentGroup<ObservableComponent, ObservableComponent, ObservableIdComponent, SimpleVisibilityShapeComponent>())
    , enableDebugRender(ShooterVisibilityDetail::ENABLE_DEBUG_RENDER_BY_DEFAULT)
    , enableDebugLog(ShooterVisibilityDetail::ENABLE_DEBUG_LOG_BY_DEFAULT)
    , framesToDebugLog(ShooterVisibilityDetail::DEBUG_LOG_PERIOD)
    , raycastCountTotal(0u)
    , raycastCountCurrFrame(0u)
    , maxRaycatCountPerFrame(0u)
{
    for (ObserverComponent* c : observerGroup->components)
    {
        AddObserver(c);
    }
    observerGroup->onComponentAdded->Connect(this, &ShooterVisibilitySystem::AddObserver);
    observerGroup->onComponentRemoved->Connect(this, &ShooterVisibilitySystem::RemoveObserver);

    for (ObservableComponent* c : characterObservableGroup->components)
    {
        AddObservable(c);
    }
    characterObservableGroup->onComponentAdded->Connect(this, &ShooterVisibilitySystem::AddObservable);
    characterObservableGroup->onComponentRemoved->Connect(this, &ShooterVisibilitySystem::RemoveObservable);

    for (ObservableComponent* c : simpleObservableGroup->components)
    {
        AddObservable(c);
    }
    simpleObservableGroup->onComponentAdded->Connect(this, &ShooterVisibilitySystem::AddObservable);
    simpleObservableGroup->onComponentRemoved->Connect(this, &ShooterVisibilitySystem::RemoveObservable);

    heightFieldComponents = scene->AquireComponentGroup<HeightFieldShapeComponent, HeightFieldShapeComponent>();
}

ShooterVisibilitySystem::~ShooterVisibilitySystem()
{
    observerGroup->onComponentAdded->Disconnect(this);
    observerGroup->onComponentRemoved->Disconnect(this);
    characterObservableGroup->onComponentAdded->Disconnect(this);
    characterObservableGroup->onComponentRemoved->Disconnect(this);
    simpleObservableGroup->onComponentAdded->Disconnect(this);
    simpleObservableGroup->onComponentRemoved->Disconnect(this);
}

Vector3 ShooterVisibilitySystem::ComputeExtrapolatedPOV(const ObserverItem& observer)
{
    const Vector3& observerPos = observer.transform->GetPosition();

    ShooterAimComponent* aimComponent = observer.comp->GetEntity()->GetComponent<ShooterAimComponent>();
    if (!aimComponent)
    {
        return observerPos;
    }

    Vector3 aimOffsetXY(SHOOTER_AIM_OFFSET.x, SHOOTER_AIM_OFFSET.y, 0.0f);

    float32 angleX = aimComponent->GetFinalAngleX();
    float32 angleZ = aimComponent->GetFinalAngleZ();
    Matrix4 rotation = Matrix4::MakeRotation(SHOOTER_CHARACTER_RIGHT, angleX) * Matrix4::MakeRotation(Vector3::UnitZ, angleZ);

    Vector3 offsetZ = Vector3(0.0f, 0.0f, SHOOTER_AIM_OFFSET.z);
    Vector3 offsetRot = aimOffsetXY * rotation;
    float32 extrapolatedZ = aimOffsetXY.Length() * ShooterVisibilityDetail::POV_EXTRAPOLATION_RATIO;
    offsetRot.z = std::max(offsetRot.z, extrapolatedZ);

    return observerPos + offsetRot + offsetZ;
}

bool ShooterVisibilitySystem::PreciseGeometricCheck(ObserverCache& observerCache, ObservableCache& observableCache)
{
    const Vector3& rayBegin = observerCache.extrapolatedPOV;
    const Vector3& rayEnd = observableCache.tracePoint;
    Vector3 direction = rayEnd - rayBegin;

    if (enableDebugRender)
    {
        visibilityRays.emplace_back(rayBegin, direction);
    }

    raycastCountCurrFrame++;

    float dist = direction.Length();
    direction /= dist;

    physx::PxHitFlags hitFlags = physx::PxHitFlag::eDEFAULT | physx::PxHitFlag::eMESH_ANY;
    physx::PxRaycastHit hit;

    for (HeightFieldCache& hf : heightFiledCaches)
    {
        physx::PxU32 hitsCount = physx::PxGeometryQuery::raycast(
        ShooterVisibilityDetail::Vector3ToPxVec3(rayBegin), ShooterVisibilityDetail::Vector3ToPxVec3(direction),
        hf.geom, hf.pose, dist, hitFlags, 1, &hit);

        if (hitsCount > 0)
        {
            return false;
        }
    }

    return true;
}

uint8 ShooterVisibilitySystem::ComputeVisibility(bool& outIsVisible, ObserverCache& observerCache, ObservableCache& observableCache)
{
    uint8 newPeriod;

    if (observerCache.entity == observableCache.entity)
    {
        outIsVisible = true;
        newPeriod = std::numeric_limits<uint8>::max();
        return newPeriod;
    }

    float32 sqrDist = DistanceSquared(observerCache.position, observableCache.position);

    if (observableCache.simpleObservable)
    {
        outIsVisible = (sqrDist <= observerCache.maxVisibilityRadiusSqr);
        newPeriod = ShooterVisibilityDetail::SIMPLE_OBSERVABLE_UDPATE_PERIOD;
        return newPeriod;
    }

    if (sqrDist < observerCache.unconditionalVisibilityRadiusSqr)
    {
        outIsVisible = true;
        newPeriod = ShooterVisibilityDetail::NEAR_ZONE_UDPATE_PERIOD;
    }
    else if (sqrDist > observerCache.maxVisibilityRadiusSqr)
    {
        outIsVisible = false;
        newPeriod = ShooterVisibilityDetail::FAR_ZONE_UDPATE_PERIOD;
    }
    else
    {
        outIsVisible = PreciseGeometricCheck(observerCache, observableCache);
        newPeriod = outIsVisible ? ShooterVisibilityDetail::RAYCAST_ZONE_VISIBLE_UDPATE_PERIOD : ShooterVisibilityDetail::RAYCAST_ZONE_INVISIBLE_UDPATE_PERIOD;
    }

    return newPeriod;
}

void ShooterVisibilitySystem::Process(float32 timeElapsed)
{
    if (enableDebugRender)
    {
        RenderHelper* renderHelper = GetScene()->GetRenderSystem()->GetDebugDrawer();
        for (Ray3& ray : visibilityRays)
        {
            renderHelper->DrawLine(ray.origin, ray.ToPoint(1.f), Color::Red);
        }
    }
}

void ShooterVisibilitySystem::PrepareHeightFieldCaches()
{
    heightFiledCaches.clear();
    for (HeightFieldShapeComponent* hfc : heightFieldComponents->components)
    {
        physx::PxShape* shape = hfc->GetPxShape();
        if (shape)
        {
            heightFiledCaches.emplace_back();
            HeightFieldCache& hf = heightFiledCaches.back();
            shape->getHeightFieldGeometry(hf.geom);
            hf.pose = shape->getLocalPose();
        }
    }
}

void ShooterVisibilitySystem::PrepareObservableCaches()
{
    for (auto item : observables)
    {
        ObservableItem& observable = item.second;

        toObserveOnCurrFrame.emplace_back();
        ObservableCache& cache = toObserveOnCurrFrame.back();

        cache.observableId = item.first;
        cache.entity = observable.transform->GetEntity();
        cache.position = observable.transform->GetPosition();

        if (observable.shape)
        {
            cache.simpleObservable = false;
            cache.tracePoint = cache.position;
            cache.tracePoint.z += observable.shape->height;
        }
        else
        {
            cache.simpleObservable = true;
        }
    }
}

void ShooterVisibilitySystem::PrepareObserverCache(ObserverCache& outCache, ObserverItem& observer)
{
    ObserverComponent* observerComp = observer.comp;
    outCache.entity = observerComp->GetEntity();
    outCache.position = observer.transform->GetPosition();
    outCache.extrapolatedPOV = ComputeExtrapolatedPOV(observer);
    outCache.maxVisibilityRadiusSqr = observerComp->maxVisibilityRadius * observerComp->maxVisibilityRadius;
    float32 uncondRad = observerComp->unconditionalVisibilityRadius;
    outCache.unconditionalVisibilityRadiusSqr = uncondRad * uncondRad;
}

void ShooterVisibilitySystem::ProcessFixed(float32 timeElapsed)
{
    DAVA_PROFILER_CPU_SCOPE("ShooterVisibilitySystem::ProcessFixed");

    if (enableDebugRender)
    {
        visibilityRays.clear();
    }

    PrepareHeightFieldCaches();
    PrepareObservableCaches();

    for (ObserverItem& observer : observers)
    {
        ObserverCache observerCache;
        PrepareObserverCache(observerCache, observer);

        ObserverComponent* observerComp = observer.comp;
        Vector<uint8>& periods = observer.updatePeriodByObservableId;

        for (ObservableCache& observableCache : toObserveOnCurrFrame)
        {
            ObservableId observableId = observableCache.observableId;
            if ((observableId + frameOffset) % periods[observableId] != 0)
            {
                continue;
            }

            bool isVisible;
            periods[observableId] = ComputeVisibility(isVisible, observerCache, observableCache);

            observerComp->SetVisible(observableId, isVisible);
        }
    }

    toObserveOnCurrFrame.clear();
    frameOffset++;

    if (enableDebugLog)
    {
        ProcessDebugLog();
    }
    raycastCountCurrFrame = 0u;
}

void ShooterVisibilitySystem::AddObserver(ObserverComponent* comp)
{
    comp->Reset();

    observers.emplace_back();
    ObserverItem& observer = observers.back();
    observer.comp = comp;
    observer.transform = comp->GetEntity()->GetComponent<TransformComponent>();
    observer.updatePeriodByObservableId.resize(MAX_OBSERVABLES_COUNT, 1);
}

void ShooterVisibilitySystem::RemoveObserver(ObserverComponent* comp)
{
    auto observerIt = std::find_if(observers.begin(), observers.end(),
                                   [comp](const ObserverItem& item) { return comp == item.comp; });

    *observerIt = std::move(observers.back());
    observers.pop_back();
}

void ShooterVisibilitySystem::AddObservable(ObservableComponent* comp)
{
    Entity* entity = comp->GetEntity();

    ObservableId observableId = entity->GetComponent<ObservableIdComponent>()->id;

    ObservableItem& observable = observables[observableId];
    observable.transform = entity->GetComponent<TransformComponent>();
    observable.shape = entity->GetComponent<CharacterVisibilityShapeComponent>();

    for (ObserverItem& observer : observers)
    {
        observer.updatePeriodByObservableId[observableId] = 1;
    }
}

void ShooterVisibilitySystem::RemoveObservable(ObservableComponent* comp)
{
    ObservableId observableId = comp->GetEntity()->GetComponent<ObservableIdComponent>()->id;
    observables.erase(observableId);

    for (ObserverItem& observer : observers)
    {
        observer.comp->SetVisible(observableId, false);
    }
}

void ShooterVisibilitySystem::EnableDebugRender(bool isEnabled)
{
    enableDebugRender = isEnabled;
}

void ShooterVisibilitySystem::EnableDebugLog(bool isEnabled)
{
    enableDebugLog = isEnabled;
}

void ShooterVisibilitySystem::ProcessDebugLog()
{
    maxRaycatCountPerFrame = std::max(maxRaycatCountPerFrame, raycastCountCurrFrame);
    raycastCountTotal += raycastCountCurrFrame;

    --framesToDebugLog;
    if (!framesToDebugLog)
    {
        framesToDebugLog = ShooterVisibilityDetail::DEBUG_LOG_PERIOD;
        float32 avgCountPerFrame = raycastCountTotal / static_cast<float32>(ShooterVisibilityDetail::DEBUG_LOG_PERIOD);
        Logger::Debug("ShooterVisibilitySystem: observers %d, observables %d, avg raycasts per frame %f, max raycasts per frame %d",
                      static_cast<DAVA::uint32>(observers.size()), static_cast<DAVA::uint32>(observables.size()),
                      avgCountPerFrame, maxRaycatCountPerFrame);
        raycastCountTotal = 0u;
        maxRaycatCountPerFrame = 0u;
    }
}
